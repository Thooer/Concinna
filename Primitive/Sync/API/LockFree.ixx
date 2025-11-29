export module Prm.Sync:LockFree;
import Lang.Element;
import :Atomic;
import :MemoryOrder;

export namespace Prm {
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
            for (;;) {
                Node* cur = Ptr(expected);
                node->next = cur;
                UIntPtr desired = Pack(node, Tag(expected) + 1);
                if (m_head.CompareExchangeWeak(expected, desired, MemoryOrder::Release, MemoryOrder::Relaxed)) break;
            }
        }
        Node* Pop() noexcept {
            UIntPtr expected = m_head.Load(MemoryOrder::Acquire);
            for (;;) {
                Node* cur = Ptr(expected);
                if (!cur) return nullptr;
                Node* next = cur->next;
                UIntPtr desired = Pack(next, Tag(expected) + 1);
                if (m_head.CompareExchangeWeak(expected, desired, MemoryOrder::Acquire, MemoryOrder::Acquire)) return cur;
            }
        }
        Node* StealAll() noexcept {
            UIntPtr expected = m_head.Load(MemoryOrder::Acquire);
            for (;;) {
                Node* cur = Ptr(expected);
                UIntPtr desired = 0;
                if (m_head.CompareExchangeWeak(expected, desired, MemoryOrder::AcqRel, MemoryOrder::Acquire)) return cur;
                if (!Ptr(expected)) return nullptr;
            }
        }
        void PushList(Node* list) noexcept {
            if (!list) return;
            Node* tail = list; while (tail->next) tail = tail->next;
            UIntPtr expected = m_head.Load(MemoryOrder::Relaxed);
            for (;;) {
                Node* cur = Ptr(expected);
                tail->next = cur;
                UIntPtr desired = Pack(list, Tag(expected) + 1);
                if (m_head.CompareExchangeWeak(expected, desired, MemoryOrder::Release, MemoryOrder::Relaxed)) break;
            }
        }
        void Reset() noexcept { m_head.Store(0, MemoryOrder::Relaxed); }
        [[nodiscard]] bool Empty() const noexcept { return Ptr(m_head.Load(MemoryOrder::Relaxed)) == nullptr; }
        [[nodiscard]] Node* Head() const noexcept { return Ptr(m_head.Load(MemoryOrder::Acquire)); }
    };
}

