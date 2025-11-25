export module Concurrency:MPMCQueue;

import Language;
import Memory;
import Platform;

import :Policy;
export namespace Concurrency {
    template<typename T, typename RP = Policy::Ebr, typename WP = Policy::Spin>
    struct MPMCQueue {
        struct Node {
            Atomic<Node*> next{nullptr};
            T value{};
        };

        Atomic<Node*> m_head{nullptr};
        Atomic<Node*> m_tail{nullptr};
        typename RP::Manager m_mgr{};
        Platform::Memory::HeapHandle m_heap{};

        MPMCQueue() noexcept {
            auto h = Platform::Memory::Heap::GetProcessDefault();
            m_heap = h;
            auto r = Platform::Memory::Heap::AllocRaw(h, sizeof(Node));
            if (!r.IsOk()) {
                m_head.Store(nullptr, MemoryOrder::Relaxed);
                m_tail.Store(nullptr, MemoryOrder::Relaxed);
                return;
            }
            void* mem = r.Value();
            auto* sentinel = new (mem) Node{};
            sentinel->next.Store(nullptr, MemoryOrder::Relaxed);
            m_head.Store(sentinel, MemoryOrder::Relaxed);
            m_tail.Store(sentinel, MemoryOrder::Relaxed);
        }

        ~MPMCQueue() noexcept {
            auto* h = m_head.Load(MemoryOrder::Acquire);
            while (h) {
                auto* n = h->next.Load(MemoryOrder::Acquire);
                h->~Node();
                (void)Platform::Memory::Heap::FreeRaw(m_heap, h);
                h = n;
            }
        }

        [[nodiscard]] Expect<void> Enqueue(const T& item) noexcept {
            return EnqueueImpl(item);
        }
        [[nodiscard]] Expect<void> Enqueue(T&& item) noexcept {
            return EnqueueImpl(Move(item));
        }

        template<typename U>
        [[nodiscard]] Expect<void> EnqueueImpl(U&& item) noexcept {
            auto h = m_heap;
            auto r = Platform::Memory::Heap::AllocRaw(h, sizeof(Node));
            if (!r.IsOk()) {
                return Expect<void>::Err(Err(StatusDomain::System(), StatusCode::Failed));
            }
            void* mem = r.Value();
            auto* node = new (mem) Node{};
            node->next.Store(nullptr, MemoryOrder::Relaxed);
            node->value = static_cast<U&&>(item);

            Backoff back{};
            for (;;) {
                auto* tail = m_tail.Load(MemoryOrder::Acquire);
                auto* next = tail->next.Load(MemoryOrder::Acquire);
                if (tail == m_tail.Load(MemoryOrder::Relaxed)) {
                    if (next == nullptr) {
                        if (tail->next.CompareExchangeWeak(next, node, MemoryOrder::Release, MemoryOrder::Relaxed)) {
                            (void)m_tail.CompareExchangeWeak(tail, node, MemoryOrder::Release, MemoryOrder::Relaxed);
                            return Expect<void>::Ok(Ok(StatusDomain::System()));
                        }
                    } else {
                        (void)m_tail.CompareExchangeWeak(tail, Launder(next), MemoryOrder::Release, MemoryOrder::Relaxed);
                    }
                }
                WP::Wait(back);
            }
        }

        [[nodiscard]] Expect<bool> Dequeue(T& out) noexcept {
            Backoff back{};
            struct EGuard { typename RP::Manager* m; typename RP::Record r; ~EGuard(){ if(m) RP::Exit(*m, r);} };
            auto rec = RP::Register(m_mgr);
            EGuard guard{ &m_mgr, rec };
            if (rec) { RP::Enter(m_mgr, rec); }
            for (;;) {
                auto* head = m_head.Load(MemoryOrder::Acquire);
                auto* tail = m_tail.Load(MemoryOrder::Relaxed);
                auto* next = head->next.Load(MemoryOrder::Acquire);
                if (head == m_head.Load(MemoryOrder::Relaxed)) {
                    if (next == nullptr) {
                        return Expect<bool>::Ok(false);
                    }
                    if (head == tail) {
                        (void)m_tail.CompareExchangeWeak(tail, Launder(next), MemoryOrder::Release, MemoryOrder::Relaxed);
                    } else {
                        out = Move(next->value);
                        if (m_head.CompareExchangeWeak(head, Launder(next), MemoryOrder::Acquire, MemoryOrder::Relaxed)) {
                            RetireNode(head);
                            RP::Advance(m_mgr);
                            RP::Collect(m_mgr);
                            return Expect<bool>::Ok(true);
                        }
                    }
                }
                back.Next();
            }
        }

        [[nodiscard]] Expect<void> EnqueueBatch(const T* items, USize count) noexcept {
            if (!items || count == 0) {
                return Expect<void>::Ok(Ok(StatusDomain::System()));
            }
            auto h = m_heap;
            Node* first = nullptr;
            Node* last = nullptr;
            for (USize i = 0; i < count; ++i) {
                auto r = Platform::Memory::Heap::AllocRaw(h, sizeof(Node));
                if (!r.IsOk()) {
                    Node* c = first;
                    while (c) {
                        Node* n = c->next.Load(MemoryOrder::Relaxed);
                        c->~Node();
                        (void)Platform::Memory::Heap::FreeRaw(h, c);
                        c = n;
                    }
                    return Expect<void>::Err(Err(StatusDomain::System(), StatusCode::Failed));
                }
                void* mem = r.Value();
                auto* node = new (mem) Node{};
                node->next.Store(nullptr, MemoryOrder::Relaxed);
                node->value = items[i];
                if (!first) { first = node; last = node; }
                else { last->next.Store(node, MemoryOrder::Relaxed); last = node; }
            }
            Backoff back{};
            for (;;) {
                auto* tail = m_tail.Load(MemoryOrder::Acquire);
                auto* next = tail->next.Load(MemoryOrder::Acquire);
                if (tail == m_tail.Load(MemoryOrder::Relaxed)) {
                    if (next == nullptr) {
                        if (tail->next.CompareExchangeWeak(next, first, MemoryOrder::Release, MemoryOrder::Relaxed)) {
                            (void)m_tail.CompareExchangeWeak(tail, last, MemoryOrder::Release, MemoryOrder::Relaxed);
                            return Expect<void>::Ok(Ok(StatusDomain::System()));
                        }
                    } else {
                        (void)m_tail.CompareExchangeWeak(tail, Launder(next), MemoryOrder::Release, MemoryOrder::Relaxed);
                    }
                }
                WP::Wait(back);
            }
        }

        void RetireNode(Node* n) noexcept {
            auto hh = m_heap.Get();
            auto reclaimer = [](void* p, void* ctx) noexcept {
                auto* node = static_cast<Node*>(p);
                Platform::Memory::HeapHandle h{ctx};
                node->~Node();
                (void)Platform::Memory::Heap::FreeRaw(h, node);
            };
            RP::Retire(m_mgr, n, hh, reclaimer);
        }
    };
}