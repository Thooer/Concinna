export module Sys.Job;
import Lang;
import Cap.Concurrency;
import Prm.Threading;
export import :Scheduler;
export import :Driver;
export import :Event;
export import :Mutex;

export namespace Sys {
    using Job = Cap::Job;
    using Counter = Cap::Counter;
    using QoS = Cap::QoS;
    using Fiber = Cap::Fiber;

    export bool JobStart(UInt32 workers) noexcept;
    export void JobStop() noexcept;
    export Status JobSubmit(void(*fn)(void*) noexcept, void* arg) noexcept;
    export Status JobSubmitPriority(void(*fn)(void*) noexcept, void* arg, UInt8 qos) noexcept;

    export void ResumeWaiters(Cap::Counter& c) noexcept;
    export void WaitForCounter(Cap::Counter& c) noexcept;
    export Status RunWithCounter(Cap::Job j, Cap::Counter& c) noexcept;
    export Status RunWithCounterPriority(Cap::Job j, Cap::Counter& c, Cap::QoS qos) noexcept;
    export Status SubmitPriority(Cap::Job j, Cap::QoS qos) noexcept;
    export void Suspend(void(*reg)(Cap::Fiber*, void*), void* ctx) noexcept;

    export template<typename Index, typename Fn>
    Status ParallelFor(Index begin, Index end, Index grain, Fn&& f) noexcept;

    export Status AwaitEvent(void* h) noexcept;
    export Status AwaitTimeout(UInt32 ms) noexcept;
    export Status Await(void* h) noexcept;
    export Status AwaitMs(UInt32 ms) noexcept;
}

namespace Sys {
    static void ResumeFiberDirect(Cap::Fiber* f) noexcept { Scheduler::Instance().ResumeFiber(f); }
    bool JobStart(UInt32 workers) noexcept {
        SetResumeFiberFn(&ResumeFiberDirect);
        (void)gDriverApi->Init();
        return Scheduler::Instance().Start(workers).Ok();
    }
    void JobStop() noexcept { (void)Scheduler::Instance().Stop(); }
    Status JobSubmit(void(*fn)(void*) noexcept, void* arg) noexcept {
        Cap::Job j{}; j.invoke = fn; j.arg = arg; return Scheduler::Instance().Submit(j);
    }
    Status JobSubmitPriority(void(*fn)(void*) noexcept, void* arg, UInt8 qos) noexcept {
        Cap::Job j{}; j.invoke = fn; j.arg = arg; j.qos = qos; return Scheduler::Instance().Submit(j);
    }
}
