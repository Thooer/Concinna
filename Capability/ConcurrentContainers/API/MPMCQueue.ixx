export module Cap.ConcurrentContainers:MPMCQueue;

import Language;
import Cap.Memory;
import Prm.Ownership;
import Prm.Sync;
 

export namespace Cap {
    using EbrRecord  = Prm::EbrRecord;
    using EbrManager = Prm::EbrManager;
    inline void* EbrHeapAlloc(USize size) noexcept { return ::operator new(size); }
    inline void  EbrHeapFree(void* p) noexcept { ::operator delete(p); }
    namespace Policy {
        struct Ebr {
            using Manager = EbrManager;
            using Record  = EbrRecord*;
            static Record Register(Manager& m) noexcept { m.SetAllocatorOps({ &EbrHeapAlloc, &EbrHeapFree }); return m.Register(); }
            static void   Enter(Manager& m, Record r) noexcept { m.Enter(r); }
            static void   Exit(Manager& m, Record r) noexcept { m.Exit(r); }
            static void   Advance(Manager& m) noexcept { m.Advance(); }
            static void   Collect(Manager& m) noexcept { m.Collect(); }
            static void   Retire(Manager& m, void* p, void* ctx, void(*f)(void*, void*) noexcept) noexcept { m.Retire(p, ctx, f); }
        };
        struct Spin { static void Wait(Prm::Backoff& b) noexcept { b.Next(); } };
    }
    template<typename T, typename RP = Cap::Policy::Ebr, typename WP = Cap::Policy::Spin>
    struct MPMCQueue {
        struct Node { Prm::Atomic<Node*> next{nullptr}; T value{}; };
        Prm::Atomic<Node*> m_head{nullptr};
        Prm::Atomic<Node*> m_tail{nullptr};
        typename RP::Manager m_mgr{};

        MPMCQueue() noexcept {
            void* mem = ::operator new(sizeof(Node), std::nothrow);
            if (!mem) { m_head.Store(nullptr, Prm::MemoryOrder::Relaxed); m_tail.Store(nullptr, Prm::MemoryOrder::Relaxed); return; }
            auto* sentinel = new (mem) Node{};
            sentinel->next.Store(nullptr, Prm::MemoryOrder::Relaxed);
            m_head.Store(sentinel, Prm::MemoryOrder::Relaxed);
            m_tail.Store(sentinel, Prm::MemoryOrder::Relaxed);
        }
        ~MPMCQueue() noexcept {
            auto* h = m_head.Load(Prm::MemoryOrder::Acquire);
            while (h) { auto* n = h->next.Load(Prm::MemoryOrder::Acquire); h->~Node(); ::operator delete(h); h = n; }
        }
        [[nodiscard]] Expect<void> Enqueue(const T& item) noexcept { return EnqueueImpl(item); }
        [[nodiscard]] Expect<void> Enqueue(T&& item) noexcept { return EnqueueImpl(Move(item)); }
        template<typename U>
        [[nodiscard]] Expect<void> EnqueueImpl(U&& item) noexcept {
            void* mem = ::operator new(sizeof(Node), std::nothrow);
            if (!mem) { return Expect<void>::Err(Err(StatusDomain::System(), StatusCode::Failed)); }
            auto* node = new (mem) Node{}; node->next.Store(nullptr, Prm::MemoryOrder::Relaxed); node->value = static_cast<U&&>(item);
            Prm::Backoff back{};
            for (;;) {
                auto* tail = m_tail.Load(Prm::MemoryOrder::Acquire);
                auto* next = tail->next.Load(Prm::MemoryOrder::Acquire);
                if (tail == m_tail.Load(Prm::MemoryOrder::Relaxed)) {
                    if (next == nullptr) {
                        if (tail->next.CompareExchangeWeak(next, node, Prm::MemoryOrder::Release, Prm::MemoryOrder::Relaxed)) {
                            (void)m_tail.CompareExchangeWeak(tail, node, Prm::MemoryOrder::Release, Prm::MemoryOrder::Relaxed);
                            return Expect<void>::Ok(Ok(StatusDomain::System()));
                        }
                    } else { (void)m_tail.CompareExchangeWeak(tail, Launder(next), Prm::MemoryOrder::Release, Prm::MemoryOrder::Relaxed); }
                }
                WP::Wait(back);
            }
        }
        [[nodiscard]] Expect<bool> Dequeue(T& out) noexcept {
            Prm::Backoff back{}; struct EGuard { typename RP::Manager* m; typename RP::Record r; ~EGuard(){ if(m) RP::Exit(*m, r);} };
            auto rec = RP::Register(m_mgr); EGuard guard{ &m_mgr, rec }; if (rec) { RP::Enter(m_mgr, rec); }
            for (;;) {
                auto* head = m_head.Load(Prm::MemoryOrder::Acquire);
                auto* tail = m_tail.Load(Prm::MemoryOrder::Relaxed);
                auto* next = head->next.Load(Prm::MemoryOrder::Acquire);
                if (head == m_head.Load(Prm::MemoryOrder::Relaxed)) {
                    if (next == nullptr) { return Expect<bool>::Ok(false); }
                    if (head == tail) { (void)m_tail.CompareExchangeWeak(tail, Launder(next), Prm::MemoryOrder::Release, Prm::MemoryOrder::Relaxed); }
                    else { out = Move(next->value); if (m_head.CompareExchangeWeak(head, Launder(next), Prm::MemoryOrder::Acquire, Prm::MemoryOrder::Relaxed)) { RetireNode(head); RP::Advance(m_mgr); RP::Collect(m_mgr); return Expect<bool>::Ok(true); } }
                }
                back.Next();
            }
        }
        [[nodiscard]] Expect<void> EnqueueBatch(const T* items, USize count) noexcept {
            if (!items || count == 0) { return Expect<void>::Ok(Ok(StatusDomain::System())); }
            Node* first = nullptr; Node* last = nullptr;
            for (USize i = 0; i < count; ++i) {
                void* mem = ::operator new(sizeof(Node), std::nothrow); if (!mem) {
                    Node* c = first; while (c) { Node* n = c->next.Load(Prm::MemoryOrder::Relaxed); c->~Node(); ::operator delete(c); c = n; }
                    return Expect<void>::Err(Err(StatusDomain::System(), StatusCode::Failed));
                }
                auto* node = new (mem) Node{}; node->next.Store(nullptr, Prm::MemoryOrder::Relaxed); node->value = items[i];
                if (!first) { first = node; last = node; } else { last->next.Store(node, Prm::MemoryOrder::Relaxed); last = node; }
            }
            Prm::Backoff back{};
            for (;;) {
                auto* tail = m_tail.Load(Prm::MemoryOrder::Acquire);
                auto* next = tail->next.Load(Prm::MemoryOrder::Acquire);
                if (tail == m_tail.Load(Prm::MemoryOrder::Relaxed)) {
                    if (next == nullptr) {
                        if (tail->next.CompareExchangeWeak(next, first, Prm::MemoryOrder::Release, Prm::MemoryOrder::Relaxed)) {
                            (void)m_tail.CompareExchangeWeak(tail, last, Prm::MemoryOrder::Release, Prm::MemoryOrder::Relaxed);
                            return Expect<void>::Ok(Ok(StatusDomain::System()));
                        }
                    } else { (void)m_tail.CompareExchangeWeak(tail, Launder(next), Prm::MemoryOrder::Release, Prm::MemoryOrder::Relaxed); }
                }
                WP::Wait(back);
            }
        }
        void RetireNode(Node* n) noexcept {
            auto reclaimer = [](void* p, void* ctx) noexcept {
                auto* node = static_cast<Node*>(p); 
                node->~Node(); 
                ::operator delete(node); 
            };
            RP::Retire(m_mgr, n, nullptr, reclaimer);
        }
    };
}
