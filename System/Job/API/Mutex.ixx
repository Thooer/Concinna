export module Sys.Job:Mutex;

import Lang;
import Cap.Concurrency;
import :Scheduler;
import Prm.Sync;

export namespace Sys {
    struct FiberMutex {
        Prm::Atomic<bool> m_locked{false};
        struct WaitNode { WaitNode* next; Cap::Fiber* fiber; };
        Prm::IntrusiveLockFreeStack<WaitNode> m_waiters{};
        [[nodiscard]] bool TryLock() noexcept;
        void Lock() noexcept;
        void Unlock() noexcept;
    };
}
