export module Memory:ConcurrentContainers;

import Language;
import Platform;

export namespace Memory {
    template<typename Node>
    struct IntrusiveLockFreeStack {
        Atomic<UIntPtr> m_head{ 0 };
        static constexpr UIntPtr kPtrMask = (sizeof(void*) == 8) ? ((static_cast<UIntPtr>(1) << 48) - 1) : ~static_cast<UIntPtr>(0);
        static constexpr UIntPtr kTagShift = (sizeof(void*) == 8) ? 48 : 0;
        static constexpr UIntPtr Pack(Node* p, UIntPtr tag) noexcept { return ((reinterpret_cast<UIntPtr>(p) & kPtrMask) | (tag << kTagShift)); }
        static constexpr Node* Ptr(UIntPtr raw) noexcept { return reinterpret_cast<Node*>(raw & kPtrMask); }
        static constexpr UIntPtr Tag(UIntPtr raw) noexcept { return (kTagShift ? (raw >> kTagShift) : 0); }
        void Push(Node* node) noexcept {
            if (!node) return;
            UIntPtr expected = m_head.Load(MemoryOrder::Relaxed);
            for (;;) { Node* cur = Ptr(expected); node->next = cur; UIntPtr desired = Pack(node, Tag(expected) + 1); if (m_head.CompareExchangeWeak(expected, desired, MemoryOrder::Release, MemoryOrder::Relaxed)) break; }
        }
        Node* Pop() noexcept {
            UIntPtr expected = m_head.Load(MemoryOrder::Acquire);
            for (;;) { Node* cur = Ptr(expected); if (!cur) return nullptr; Node* next = cur->next; UIntPtr desired = Pack(next, Tag(expected) + 1); if (m_head.CompareExchangeWeak(expected, desired, MemoryOrder::Acquire, MemoryOrder::Acquire)) return cur; }
        }
        Node* StealAll() noexcept {
            UIntPtr expected = m_head.Load(MemoryOrder::Acquire);
            for (;;) { Node* cur = Ptr(expected); UIntPtr desired = 0; if (m_head.CompareExchangeWeak(expected, desired, MemoryOrder::AcqRel, MemoryOrder::Acquire)) return cur; if (!Ptr(expected)) return nullptr; }
        }
        void PushList(Node* list) noexcept {
            if (!list) return;
            Node* tail = list; while (tail->next) tail = tail->next;
            UIntPtr expected = m_head.Load(MemoryOrder::Relaxed);
            for (;;) { Node* cur = Ptr(expected); tail->next = cur; UIntPtr desired = Pack(list, Tag(expected) + 1); if (m_head.CompareExchangeWeak(expected, desired, MemoryOrder::Release, MemoryOrder::Relaxed)) break; }
        }
        void Reset() noexcept { m_head.Store(0, MemoryOrder::Relaxed); }
        [[nodiscard]] bool Empty() const noexcept { return Ptr(m_head.Load(MemoryOrder::Relaxed)) == nullptr; }
        [[nodiscard]] Node* Head() const noexcept { return Ptr(m_head.Load(MemoryOrder::Acquire)); }
    };

    struct EbrRecord {
        EbrRecord* next;
        Atomic<UInt64> epoch{0};
        Atomic<UInt32> active{0};
    };

    struct EbrNode {
        EbrNode* next;
        void* ptr;
        void* ctx;
        void(*reclaim)(void*, void*) noexcept;
        UInt64 epoch;
    };

    struct EbrManager {
        Atomic<UInt64> m_epoch{0};
        IntrusiveLockFreeStack<EbrRecord> m_records{};
        IntrusiveLockFreeStack<EbrNode> m_retired0{};
        IntrusiveLockFreeStack<EbrNode> m_retired1{};
        IntrusiveLockFreeStack<EbrNode> m_retired2{};

        void Enter(EbrRecord* r) noexcept {
            const auto e = m_epoch.Load(MemoryOrder::Acquire);
            r->epoch.Store(e, MemoryOrder::Relaxed);
            r->active.Store(1u, MemoryOrder::Release);
        }
        void Exit(EbrRecord* r) noexcept { r->active.Store(0u, MemoryOrder::Release); }

        void Advance() noexcept {
            const auto e = m_epoch.Load(MemoryOrder::Relaxed);
            m_epoch.Store(e + 1u, MemoryOrder::Release);
        }
        void Retire(void* p, void* ctx, void(*reclaimer)(void*, void*) noexcept) noexcept {
            if (!p || !reclaimer) return;
            auto h = Platform::Memory::Heap::GetProcessDefault();
            auto mn = Platform::Memory::Heap::AllocRaw(h, sizeof(EbrNode));
            if (!mn.IsOk()) return;
            void* nm = mn.Value();
            auto* n = new (nm) EbrNode{};
            n->ptr = p;
            n->ctx = ctx;
            n->reclaim = reclaimer;
            n->epoch = m_epoch.Load(MemoryOrder::Relaxed);
            const auto idx = static_cast<UInt64>(n->epoch % 3u);
            if (idx == 0u) { m_retired0.Push(n); }
            else if (idx == 1u) { m_retired1.Push(n); }
            else { m_retired2.Push(n); }
        }
        void Collect() noexcept {
            const auto e = m_epoch.Load(MemoryOrder::Acquire);
            if (e < 2u) return;
            auto minEpoch = e;
            for (auto* r = m_records.Head(); r; r = r->next) {
                const auto a = r->active.Load(MemoryOrder::Acquire);
                if (a) {
                    const auto re = r->epoch.Load(MemoryOrder::Acquire);
                    if (re < minEpoch) minEpoch = re;
                }
            }
            if (minEpoch < (e - 1u)) return;
            const auto idx = static_cast<UInt64>((e + 1u) % 3u);
            auto h = Platform::Memory::Heap::GetProcessDefault();
            constexpr UInt32 kCollectBatch = 1024u;
            UInt32 processed = 0u;
            if (idx == 0u) {
                while (processed < kCollectBatch) {
                    auto* n = m_retired0.Pop();
                    if (!n) break;
                    if (n->reclaim) { n->reclaim(n->ptr, n->ctx); }
                    n->~EbrNode();
                    (void)Platform::Memory::Heap::FreeRaw(h, n);
                    ++processed;
                }
            } else if (idx == 1u) {
                while (processed < kCollectBatch) {
                    auto* n = m_retired1.Pop();
                    if (!n) break;
                    if (n->reclaim) { n->reclaim(n->ptr, n->ctx); }
                    n->~EbrNode();
                    (void)Platform::Memory::Heap::FreeRaw(h, n);
                    ++processed;
                }
            } else {
                while (processed < kCollectBatch) {
                    auto* n = m_retired2.Pop();
                    if (!n) break;
                    if (n->reclaim) { n->reclaim(n->ptr, n->ctx); }
                    n->~EbrNode();
                    (void)Platform::Memory::Heap::FreeRaw(h, n);
                    ++processed;
                }
            }
        }

        EbrRecord* Register() noexcept {
            thread_local EbrRecord* t = nullptr;
            if (t) return t;
            auto h = Platform::Memory::Heap::GetProcessDefault();
            auto mr = Platform::Memory::Heap::AllocRaw(h, sizeof(EbrRecord));
            if (!mr.IsOk()) return nullptr;
            void* nm = mr.Value();
            t = new (nm) EbrRecord{};
            m_records.Push(t);
            return t;
        }

        struct EpochGuard {
            EbrManager* m;
            EbrRecord* r;
            EpochGuard(EbrManager& mgr) noexcept : m(&mgr), r(mgr.Register()) { if (r) m->Enter(r); }
            ~EpochGuard() noexcept { if (r) m->Exit(r); }
            EpochGuard(const EpochGuard&) = delete;
            EpochGuard& operator=(const EpochGuard&) = delete;
            EpochGuard(EpochGuard&& o) noexcept : m(o.m), r(o.r) { o.m = nullptr; o.r = nullptr; }
            EpochGuard& operator=(EpochGuard&& o) noexcept { if (this != &o) { m = o.m; r = o.r; o.m = nullptr; o.r = nullptr; } return *this; }
        };
    };
}