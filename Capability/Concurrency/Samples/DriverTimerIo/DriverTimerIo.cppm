module;
export module Concurrency.DriverTimerIo;
import Language;
import Platform;
import Concurrency;

static void SleepTask(void* p) noexcept {
    UInt32 ms = *static_cast<UInt32*>(p);
    Concurrency::SleepMs(ms);
}

static void EventWaitTask(void* p) noexcept {
    auto* ev = static_cast<Concurrency::FiberEvent*>(p);
    ev->WaitFor(50);
}

static void IoAwaitTask(void* p) noexcept {
    auto* hv = static_cast<Platform::EventHandle*>(p);
    auto reg = +[](Concurrency::Fiber* f, void* ctx) noexcept {
        auto* h = static_cast<Platform::EventHandle*>(ctx);
        (void)Concurrency::gDriverApi->AddEvent(*h, f);
    };
    Concurrency::Suspend(reg, hv);
}

export int main() {
    auto& sch = Concurrency::Scheduler::Instance();
    (void)sch.Start(4);

    Concurrency::Counter c{};
    c.Reset(6);

    auto h = Platform::Memory::Heap::GetProcessDefault();
    auto rn = Platform::Memory::Heap::AllocRaw(h, sizeof(UInt32));
    auto* ms = static_cast<UInt32*>(rn.Value());
    *ms = 20;
    Concurrency::Job j1{}; j1.invoke = &SleepTask; j1.arg = ms;
    (void)Concurrency::RunWithCounter(j1, c);

    Concurrency::FiberEvent ev{};
    Concurrency::Job j2{}; j2.invoke = &EventWaitTask; j2.arg = &ev;
    (void)Concurrency::RunWithCounter(j2, c);

    Concurrency::Job j3{}; j3.invoke = +[](void* p) noexcept { Platform::ThreadSleepMs(10); static_cast<Concurrency::FiberEvent*>(p)->Signal(); };
    j3.arg = &ev; (void)Concurrency::RunWithCounter(j3, c);

    auto re = Platform::EventCreate(false, false);
    Platform::EventHandle e{}; if (re.IsOk()) e = re.Value();
    Concurrency::Job j4{}; j4.invoke = &IoAwaitTask; j4.arg = &e;
    (void)Concurrency::RunWithCounter(j4, c);

    Concurrency::Job j5{}; j5.invoke = +[](void* p) noexcept { Platform::ThreadSleepMs(30); (void)Platform::EventSignal(*static_cast<Platform::EventHandle*>(p)); };
    j5.arg = &e; (void)Concurrency::RunWithCounter(j5, c);

    Concurrency::Job j6{}; j6.invoke = +[](void* p) noexcept { (void)p; Concurrency::SleepMs(5); };
    (void)Concurrency::RunWithCounter(j6, c);

    Concurrency::WaitForCounter(c);
    (void)sch.Stop();
    return 0;
}