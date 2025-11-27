export module System.Job:Mutex;

import Language;
import Memory;
import Cap.Concurrency:Fiber;
import System.Job:Scheduler;
import Prm.Sync:LockFree;

export namespace Sys {
    struct FiberMutex {
        Atomic<bool> m_locked{false};
        struct WaitNode { WaitNode* next; Cap::Fiber* fiber; };
        Prm::IntrusiveLockFreeStack<WaitNode> m_waiters{};
        [[nodiscard]] bool TryLock() noexcept;
        void Lock() noexcept;
        void Unlock() noexcept;
    };
}
