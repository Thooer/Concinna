import Language;
import Platform;
import Memory;
import Concurrency;
import <cstdio>;

static void AllocThunk(void* p) noexcept {
    (void)p;
}

extern "C" int main() {
    using namespace Concurrency;
    printf("Begin\n");
    auto& sch = Scheduler::Instance();
    (void)sch.Start(0u);
    printf("Scheduler started\n");
    Concurrency::Counter c{};
    UInt32 tasks = 1u;
    c.Reset(tasks);
    auto h = Platform::Memory::Heap::GetProcessDefault();
    for (UInt32 i = 0; i < tasks; ++i) {
        auto rn = Platform::Memory::Heap::AllocRaw(h, sizeof(UInt32));
        if (!rn.IsOk()) continue;
        auto* pv = static_cast<UInt32*>(rn.Value());
        *pv = i;
        Concurrency::Job j{}; j.invoke = &AllocThunk; j.arg = pv;
        (void)Concurrency::RunWithCounter(j, c);
    }
    auto start = Platform::Time::Now();
    printf("Counter before wait=%u\n", c.Value());
    Concurrency::WaitForCounter(c);
    printf("Counter after wait=%u\n", c.Value());
    printf("Wait complete\n");
    auto end = Platform::Time::Now();
    auto dt = Platform::Time::DeltaSeconds(start, end);
    printf("FiberStress tasks=%u dt=%.6f s tps=%.0f\n", tasks, dt, tasks / dt);
    {
        auto& sch2 = Scheduler::Instance();
        auto wc = sch2.WorkerCount();
        printf("workers=%u\n", wc);
        for (UInt32 i = 0; i < wc; ++i) {
            Scheduler::WorkerMetrics m{}; bool ok = sch2.GetWorkerMetrics(i, m);
            if (ok) {
                printf("W%u run=%.3fms count=%llu steal=%.3fms hit=%llu wait=%.3fms\n",
                    i,
                    (double)m.runNs / 1e6,
                    (unsigned long long)m.runCount,
                    (double)m.stealNs / 1e6,
                    (unsigned long long)m.stealHit,
                    (double)m.waitNs / 1e6);
            }
        }
    }
    (void)sch.Stop();
    printf("Stopped\n");
    return 0;
}
