module Sys;
import Lang;
import Cap.Memory;
import Prm.Ownership;
import Cap.Concurrency;
import :Scheduler;
import :Event;
import :Driver;

namespace Sys {
    void FiberEvent::Signal() noexcept {
        m_signaled.Store(true, Prm::MemoryOrder::Release);
        auto* n = m_waiters.Pop();
        if (n) {
            if (!n->canceled.Load(Prm::MemoryOrder::Acquire)) { Scheduler::Instance().ResumeFiber(n->fiber); }
            ::operator delete(n);
        }
    }

    void FiberEvent::Broadcast() noexcept {
        m_signaled.Store(true, Prm::MemoryOrder::Release);
        WaitNode* n = nullptr;
        while ((n = m_waiters.Pop()) != nullptr) {
            if (!n->canceled.Load(Prm::MemoryOrder::Acquire)) { Scheduler::Instance().ResumeFiber(n->fiber); }
            ::operator delete(n);
        }
    }

    void FiberEvent::Wait() noexcept {
        if (m_signaled.Load(Prm::MemoryOrder::Acquire)) return;
        auto reg = +[](Cap::Fiber* f, void* ctx) noexcept {
            auto* self = static_cast<FiberEvent*>(ctx);
            void* mem = ::operator new(sizeof(WaitNode));
            if (!mem) return;
            auto* n = new (mem) WaitNode{};
            n->fiber = f;
            self->m_waiters.Push(n);
        };
        Sys::Suspend(reg, this);
    }

    static void OnTimeoutWaitNode(void* p) noexcept {
        auto* n = static_cast<FiberEvent::WaitNode*>(p);
        n->canceled.Store(true, Prm::MemoryOrder::Release);
        if (n->fiber) Scheduler::Instance().ResumeFiber(static_cast<Cap::Fiber*>(n->fiber));
    }

    void FiberEvent::WaitFor(UInt32 timeoutMs) noexcept {
        if (m_signaled.Load(Prm::MemoryOrder::Acquire)) return;
        struct WaitPack { FiberEvent* self; UInt32 ms; };
        WaitPack pack{ this, timeoutMs };
        auto reg = +[](Cap::Fiber* f, void* ctx) noexcept {
            auto* pp = static_cast<WaitPack*>(ctx);
            auto* self = pp->self;
            void* mem = ::operator new(sizeof(WaitNode));
            if (!mem) return;
            auto* n = new (mem) WaitNode{};
            n->fiber = f;
            self->m_waiters.Push(n);
            if (pp->ms != 0u) { (void)gDriverApi->AddTimeout(&OnTimeoutWaitNode, n, pp->ms); }
        };
        Sys::Suspend(reg, &pack);
    }

    void FiberEvent::Reset() noexcept { m_signaled.Store(false, Prm::MemoryOrder::Relaxed); }
}
