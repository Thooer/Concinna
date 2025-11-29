export module Sys:Event;

import Lang;
import Cap.Concurrency;
import :Scheduler;
import Prm.Sync;
export namespace Sys {
    struct FiberEvent {
        Prm::Atomic<bool> m_signaled{false};
        struct WaitNode { WaitNode* next; Cap::Fiber* fiber; Prm::Atomic<bool> canceled{false}; };
        Prm::IntrusiveLockFreeStack<WaitNode> m_waiters{};
        void Signal() noexcept;
        void Broadcast() noexcept;
        void Wait() noexcept;
        void WaitFor(UInt32 timeoutMs) noexcept;
        void Reset() noexcept;
    };
}
