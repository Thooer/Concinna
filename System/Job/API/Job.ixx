export module System.Job;
import Language;
import System.Job:Scheduler;
import Cap.Concurrency:Job;

export namespace Sys {
    export bool JobStart(Language::UInt32 workers) noexcept;
    export void JobStop() noexcept;
    export Language::Status JobSubmit(void(*fn)(void*) noexcept, void* arg) noexcept;
    export Language::Status JobSubmitPriority(void(*fn)(void*) noexcept, void* arg, Language::UInt8 qos) noexcept;
}

namespace Sys {
    bool JobStart(Language::UInt32 workers) noexcept {
        return Sys::Scheduler::Instance().Start(workers).Ok();
    }
    void JobStop() noexcept {
        (void)Sys::Scheduler::Instance().Stop();
    }
    Language::Status JobSubmit(void(*fn)(void*) noexcept, void* arg) noexcept {
        Cap::Job j{}; j.invoke = fn; j.arg = arg;
        return Sys::Scheduler::Instance().Submit(j);
    }
    Language::Status JobSubmitPriority(void(*fn)(void*) noexcept, void* arg, Language::UInt8 qos) noexcept {
        Cap::Job j{}; j.invoke = fn; j.arg = arg; j.qos = qos;
        return Sys::Scheduler::Instance().Submit(j);
    }
}
