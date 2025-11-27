export module Prm.Sync:EBR;
import Element;
import :Atomic;
import :MemoryOrder;
import :LockFree;

export namespace Prm {
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

    struct EbrAllocatorOps {
        void* (*alloc)(USize);
        void (*free)(void*);
    };

    struct EbrManager {
        Atomic<UInt64> m_epoch{0};
        IntrusiveLockFreeStack<EbrRecord> m_records{};
        IntrusiveLockFreeStack<EbrNode> m_retired0{};
        IntrusiveLockFreeStack<EbrNode> m_retired1{};
        IntrusiveLockFreeStack<EbrNode> m_retired2{};
        EbrAllocatorOps m_ops{nullptr, nullptr};

        void SetAllocatorOps(EbrAllocatorOps ops) noexcept { m_ops = ops; }
        [[nodiscard]] bool HasAllocator() const noexcept { return m_ops.alloc != nullptr && m_ops.free != nullptr; }

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
            if (!p || !reclaimer || !HasAllocator()) return;
            void* nm = m_ops.alloc(sizeof(EbrNode));
            if (!nm) return;
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
            if (!HasAllocator()) return;
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
            constexpr UInt32 kCollectBatch = 1024u;
            UInt32 processed = 0u;
            auto drain = [this, &processed, kCollectBatch](IntrusiveLockFreeStack<EbrNode>& s) noexcept {
                while (processed < kCollectBatch) {
                    auto* n = s.Pop();
                    if (!n) break;
                    if (n->reclaim) { n->reclaim(n->ptr, n->ctx); }
                    n->~EbrNode();
                    m_ops.free(n);
                    ++processed;
                }
            };
            if (idx == 0u) drain(m_retired0);
            else if (idx == 1u) drain(m_retired1);
            else drain(m_retired2);
        }

        EbrRecord* Register() noexcept {
            if (!HasAllocator()) return nullptr;
            thread_local EbrRecord* t = nullptr;
            if (t) return t;
            void* nm = m_ops.alloc(sizeof(EbrRecord));
            if (!nm) return nullptr;
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

