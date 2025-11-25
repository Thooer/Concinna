export module Prm.Sync:SpinLock;
import Prm.Element;
import :Atomic;
import :Backoff;
import :MemoryOrder;

export namespace Prm {
    struct SpinLock {
        Atomic<bool> m_locked{false};

        constexpr SpinLock() noexcept = default;
        SpinLock(const SpinLock&) = delete;
        SpinLock& operator=(const SpinLock&) = delete;

        void Lock() noexcept {
            Backoff b{};
            for (;;) {
                if (!m_locked.Load(MemoryOrder::Relaxed)) {
                    bool expected = false;
                    if (m_locked.CompareExchangeWeak(expected, true, MemoryOrder::Acquire, MemoryOrder::Relaxed)) {
                        return;
                    }
                }
                b.Next();
            }
        }

        [[nodiscard]] bool TryLock() noexcept {
            bool expected = false;
            return m_locked.CompareExchangeStrong(expected, true, MemoryOrder::Acquire, MemoryOrder::Relaxed);
        }

        void Unlock() noexcept { m_locked.Store(false, MemoryOrder::Release); }
    };
}

