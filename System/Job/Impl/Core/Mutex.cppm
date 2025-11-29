module Sys;
import Lang;
import Cap.Memory;
import Prm.Ownership;
import Sys;
import Cap.Concurrency;

namespace Sys {
    bool FiberMutex::TryLock() noexcept {
        bool expected = false;
        return m_locked.CompareExchangeStrong(expected, true, Prm::MemoryOrder::Acquire, Prm::MemoryOrder::Relaxed);
    }

    void FiberMutex::Lock() noexcept {
        if (TryLock()) return;
        Prm::Backoff back{};
        for (int i = 0; i < 64; ++i) {
            if (TryLock()) return;
            back.Next();
        }
        auto reg = +[](Cap::Fiber* f, void* ctx) noexcept {
            auto* self = static_cast<FiberMutex*>(ctx);
            void* mem = ::operator new(sizeof(WaitNode));
            if (!mem) return;
            auto* n = new (mem) WaitNode{};
            n->fiber = f;
            self->m_waiters.Push(n);
        };
        Sys::Suspend(reg, this);
        for (;;) {
            if (TryLock()) return;
            back.Next();
        }
    }

    void FiberMutex::Unlock() noexcept {
        m_locked.Store(false, Prm::MemoryOrder::Release);
        auto* n = m_waiters.Pop();
        if (n) {
            Scheduler::Instance().ResumeFiber(n->fiber);
            ::operator delete(n);
        }
    }
}
