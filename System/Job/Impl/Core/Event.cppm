module System.Job;
import Language;
import Memory;
import Prm.Ownership:Memory;
import System.Job:Event;
import Cap.Concurrency:Fiber;
import System.Job:Scheduler;
import System.Job:Driver;

namespace Sys {
    void FiberEvent::Signal() noexcept {
        m_signaled.Store(true, MemoryOrder::Release);
        auto* n = m_waiters.Pop();
        if (n) {
            if (!n->canceled.Load(MemoryOrder::Acquire)) { Scheduler::Instance().ResumeFiber(n->fiber); }
            auto h = Prm::Heap::GetProcessDefault();
            (void)Prm::Heap::FreeRaw(h, n);
        }
    }

    void FiberEvent::Broadcast() noexcept {
        m_signaled.Store(true, MemoryOrder::Release);
        WaitNode* n = nullptr;
        while ((n = m_waiters.Pop()) != nullptr) {
            if (!n->canceled.Load(MemoryOrder::Acquire)) { Scheduler::Instance().ResumeFiber(n->fiber); }
            auto h = Prm::Heap::GetProcessDefault();
            (void)Prm::Heap::FreeRaw(h, n);
        }
    }

    void FiberEvent::Wait() noexcept {
        if (m_signaled.Load(MemoryOrder::Acquire)) return;
        auto reg = +[](Cap::Fiber* f, void* ctx) noexcept {
            auto* self = static_cast<FiberEvent*>(ctx);
            auto h = Prm::Heap::GetProcessDefault();
            auto rn = Prm::Heap::AllocRaw(h, sizeof(WaitNode));
            if (!rn.IsOk()) return;
            void* mem = rn.Value();
            auto* n = new (mem) WaitNode{};
            n->fiber = f;
            self->m_waiters.Push(n);
        };
        Sys::Suspend(reg, this);
    }

    static void OnTimeoutWaitNode(void* p) noexcept {
        auto* n = static_cast<FiberEvent::WaitNode*>(p);
        n->canceled.Store(true, MemoryOrder::Release);
        if (n->fiber) Scheduler::Instance().ResumeFiber(static_cast<Cap::Fiber*>(n->fiber));
    }

    void FiberEvent::WaitFor(UInt32 timeoutMs) noexcept {
        if (m_signaled.Load(MemoryOrder::Acquire)) return;
        struct WaitPack { FiberEvent* self; UInt32 ms; };
        WaitPack pack{ this, timeoutMs };
        auto reg = +[](Cap::Fiber* f, void* ctx) noexcept {
            auto* pp = static_cast<WaitPack*>(ctx);
            auto* self = pp->self;
            auto h = Prm::Heap::GetProcessDefault();
            auto rn = Prm::Heap::AllocRaw(h, sizeof(WaitNode));
            if (!rn.IsOk()) return;
            void* mem = rn.Value();
            auto* n = new (mem) WaitNode{};
            n->fiber = f;
            self->m_waiters.Push(n);
            if (pp->ms != 0u) { (void)gDriverApi->AddTimeout(&OnTimeoutWaitNode, n, pp->ms); }
        };
        Sys::Suspend(reg, &pack);
    }

    void FiberEvent::Reset() noexcept { m_signaled.Store(false, MemoryOrder::Relaxed); }
}
