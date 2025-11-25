export module Concurrency:Mutex;

import Language;
import Memory;
import Platform;
import :Fiber;
import :Scheduler;

export namespace Concurrency {
    struct FiberMutex {
        Atomic<bool> m_locked{false};
        struct WaitNode { WaitNode* next; Fiber* fiber; };
        Memory::IntrusiveLockFreeStack<WaitNode> m_waiters{};
        [[nodiscard]] bool TryLock() noexcept;
        void Lock() noexcept;
        void Unlock() noexcept;
    };
}