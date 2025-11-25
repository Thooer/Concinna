export module Concurrency:Event;

import Language;
import Memory;
import Platform;
import :Fiber;
import :Scheduler;

export namespace Concurrency {
    struct FiberEvent {
        Atomic<bool> m_signaled{false};
        struct WaitNode { WaitNode* next; Fiber* fiber; Atomic<bool> canceled{false}; };
        Memory::IntrusiveLockFreeStack<WaitNode> m_waiters{};
        void Signal() noexcept;
        void Broadcast() noexcept;
        void Wait() noexcept;
        void WaitFor(UInt32 timeoutMs) noexcept;
        void Reset() noexcept;
    };
}