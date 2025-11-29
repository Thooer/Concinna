module;
export module Sys;
import Lang;
import :Scheduler;
export import :Scheduler;
export import :Driver;
export import :Event;
export import :Mutex;
import Cap.Concurrency;

export namespace Sys {
    export bool JobStart(UInt32 workers) noexcept;
    export void JobStop() noexcept;
    export Status JobSubmit(void(*fn)(void*) noexcept, void* arg) noexcept;
    export Status JobSubmitPriority(void(*fn)(void*) noexcept, void* arg, UInt8 qos) noexcept;
}

namespace Sys {
    bool JobStart(UInt32 workers) noexcept {
        return Sys::Scheduler::Instance().Start(workers).Ok();
    }
    void JobStop() noexcept {
        (void)Sys::Scheduler::Instance().Stop();
    }
    Status JobSubmit(void(*fn)(void*) noexcept, void* arg) noexcept {
        Cap::Job j{}; j.invoke = fn; j.arg = arg;
        return Sys::Scheduler::Instance().Submit(j);
    }
    Status JobSubmitPriority(void(*fn)(void*) noexcept, void* arg, UInt8 qos) noexcept {
        Cap::Job j{}; j.invoke = fn; j.arg = arg; j.qos = qos;
        return Sys::Scheduler::Instance().Submit(j);
    }
}
