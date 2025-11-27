export module System.Job:Event;

import Language;
import Memory;
import Cap.Concurrency:Fiber;
import System.Job:Scheduler;
import Prm.Sync:LockFree;

export namespace Sys {
    struct FiberEvent {
        Atomic<bool> m_signaled{false};
        struct WaitNode { WaitNode* next; Cap::Fiber* fiber; Atomic<bool> canceled{false}; };
        Prm::IntrusiveLockFreeStack<WaitNode> m_waiters{};
        void Signal() noexcept;
        void Broadcast() noexcept;
        void Wait() noexcept;
        void WaitFor(UInt32 timeoutMs) noexcept;
        void Reset() noexcept;
    };
}
