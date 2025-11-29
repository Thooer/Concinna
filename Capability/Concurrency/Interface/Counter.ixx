export module Cap.Concurrency:Counter;

import Lang;
import Prm.Sync;
import Cap.Memory;

export namespace Cap {
    struct Counter {
        Atomic<UInt32> m_value{0};
        struct WaitNode { WaitNode* next; void* fiber; };
        Prm::IntrusiveLockFreeStack<WaitNode> m_waiters{};
        void Reset(UInt32 v = 0) noexcept;
        void Add(UInt32 v) noexcept;
        UInt32 Dec() noexcept;
        void SignalComplete() noexcept;
        [[nodiscard]] UInt32 Value() const noexcept;
        bool RegisterWait(void* fiberPtr) noexcept;
        void DrainWaiters(void(*resume)(void*)) noexcept;
    };

    [[nodiscard]] Status WaitForZero(Counter& c, UInt32 timeoutMs) noexcept;
    void NotifyCounter(Counter& c) noexcept;
}
