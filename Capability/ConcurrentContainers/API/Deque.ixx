export module Cap.ConcurrentContainers:Deque;

import Lang;
import Cap.Memory;
import Prm.Sync;

export namespace Cap {
    [[nodiscard]] constexpr bool IsPowerOfTwo(USize v) noexcept { return v > 0 && (v & (v - 1)) == 0; }
    template<typename T>
    struct ChaseLevDeque {
        T* m_buffer{nullptr};
        USize m_mask{0};
        alignas(64) Prm::Atomic<USize> m_top{0};
        alignas(64) USize m_bottom{0};

        bool Init(void* storage, USize capacity) noexcept {
            if (!storage || capacity == 0) return false;
            if (!IsPowerOfTwo(capacity)) return false;
            m_buffer = static_cast<T*>(storage);
            m_mask = capacity - 1;
            m_top.Store(0, Prm::MemoryOrder::Relaxed);
            m_bottom = 0;
            return true;
        }

        [[nodiscard]] USize Size() const noexcept {
            const auto t = m_top.Load(Prm::MemoryOrder::Acquire);
            const auto b = m_bottom;
            return (b >= t) ? (b - t) : 0;
        }

        [[nodiscard]] bool PushBottom(const T& x) noexcept {
            const auto b = m_bottom;
            const auto t = m_top.Load(Prm::MemoryOrder::Acquire);
            if ((b - t) >= (m_mask + 1)) return false;
            m_buffer[b & m_mask] = x;
            Prm::ThreadFence(Prm::MemoryOrder::Release);
            m_bottom = b + 1;
            return true;
        }
        [[nodiscard]] bool PushBottom(T&& x) noexcept {
            const auto b = m_bottom;
            const auto t = m_top.Load(Prm::MemoryOrder::Acquire);
            if ((b - t) >= (m_mask + 1)) return false;
            m_buffer[b & m_mask] = Move(x);
            Prm::ThreadFence(Prm::MemoryOrder::Release);
            m_bottom = b + 1;
            return true;
        }

        [[nodiscard]] bool PopBottom(T& out) noexcept {
            auto b = m_bottom;
            if (b == 0) return false;
            b = b - 1;
            m_bottom = b;
            Prm::ThreadFence(Prm::MemoryOrder::Acquire);
            auto t = m_top.Load(Prm::MemoryOrder::Acquire);
            if (t <= b) {
                out = Move(m_buffer[b & m_mask]);
                if (t != b) {
                    return true;
                }
                const auto tp1 = t + 1;
                if (m_top.CompareExchangeStrong(t, tp1, Prm::MemoryOrder::Acquire, Prm::MemoryOrder::Relaxed)) {
                    m_bottom = tp1;
                    return true;
                } else {
                    m_bottom = tp1;
                    return false;
                }
            } else {
                m_bottom = b + 1;
                return false;
            }
        }

        [[nodiscard]] bool Steal(T& out) noexcept {
            auto t = m_top.Load(Prm::MemoryOrder::Acquire);
            Prm::ThreadFence(Prm::MemoryOrder::Acquire);
            const auto b = m_bottom;
            if (t < b) {
                out = m_buffer[t & m_mask];
                const auto tp1 = t + 1;
                if (m_top.CompareExchangeStrong(t, tp1, Prm::MemoryOrder::Acquire, Prm::MemoryOrder::Relaxed)) {
                    return true;
                }
            }
            return false;
        }
    };
}
