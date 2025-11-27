module System.Job;
import Language;
import Memory;
import Prm.Ownership:Memory;
import System.Job:Mutex;
import Cap.Concurrency:Fiber;
import System.Job:Scheduler;

namespace Sys {
    bool FiberMutex::TryLock() noexcept {
        bool expected = false;
        return m_locked.CompareExchangeStrong(expected, true, MemoryOrder::Acquire, MemoryOrder::Relaxed);
    }

    void FiberMutex::Lock() noexcept {
        if (TryLock()) return;
        Backoff back{};
        for (int i = 0; i < 64; ++i) {
            if (TryLock()) return;
            back.Next();
        }
        auto reg = +[](Cap::Fiber* f, void* ctx) noexcept {
            auto* self = static_cast<FiberMutex*>(ctx);
            auto h = Prm::Heap::GetProcessDefault();
            auto rn = Prm::Heap::AllocRaw(h, sizeof(WaitNode));
            if (!rn.IsOk()) return;
            void* mem = rn.Value();
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
        m_locked.Store(false, MemoryOrder::Release);
        auto* n = m_waiters.Pop();
        if (n) {
            Scheduler::Instance().ResumeFiber(n->fiber);
            auto h = Prm::Heap::GetProcessDefault();
            (void)Prm::Heap::FreeRaw(h, n);
        }
    }
}
