import Language;
import Platform;
import System;
import Memory;
import Concurrency;
import <cstdio>;

struct PressureCtx { Int64 endNs; Atomic<UInt64>* counter; UInt64 seed; UInt32 id; };
static void ThreadEntry(void* p) noexcept {
    auto* ctx = static_cast<PressureCtx*>(p);
    UInt64 x = ctx->seed ^ 0x9e3779b97f4a7c15ull;
    for (;;) {
        auto now = Platform::Time::Now();
        if (now >= ctx->endNs) break;
        x ^= (x << 13);
        x ^= (x >> 7);
        x ^= (x << 17);
        x = x * 0xbf58476d1ce4e5b9ull + 0x94d049bb133111ebull;
        x ^= (x >> 33);
        (void)ctx->counter->FetchAdd(64u, MemoryOrder::Relaxed);
    }
    (void)Platform::Memory::Heap::FreeRaw(Platform::Memory::Heap::GetProcessDefault(), ctx);
}

extern "C" int main() {
    auto& sch = Concurrency::Scheduler::Instance();
    auto ci = System::SystemInfo::Cpu();
    UInt32 workers = ci.logicalCores ? ci.logicalCores : 1u;
    (void)sch.Start(workers);
    auto h = Platform::Memory::Heap::GetProcessDefault();
    auto startNs = Platform::Time::Now();
    auto endNs = startNs + 60'000'000'000ll;
    printf("HighPressure start: %u workers, 60s\n", workers);
    auto r = Platform::Memory::Heap::AllocRaw(h, sizeof(Atomic<UInt64>) * workers);
    if (!r.IsOk()) return 1;
    auto* counters = static_cast<Atomic<UInt64>*>(r.Value());
    for (UInt32 i = 0; i < workers; ++i) { new (counters + i) Atomic<UInt64>(0u); }
    Concurrency::Counter c{}; c.Reset(workers);
    for (UInt32 i = 0; i < workers; ++i) {
        auto rn = Platform::Memory::Heap::AllocRaw(h, sizeof(PressureCtx));
        if (!rn.IsOk()) continue;
        auto* ctx = static_cast<PressureCtx*>(rn.Value());
        ctx->endNs = endNs;
        ctx->counter = counters + i;
        ctx->seed = static_cast<UInt64>(i + 1u);
        ctx->id = i;
        Concurrency::Job j{}; j.invoke = &ThreadEntry; j.arg = ctx; j.qos = static_cast<UInt8>(Concurrency::QoS::High);
        (void)Concurrency::RunWithCounter(j, c);
    }
    UInt64 prev = 0u;
    for (UInt32 sec = 1; sec <= 60u; ++sec) {
        Platform::ThreadSleepMs(1000);
        UInt64 sum = 0u;
        for (UInt32 i = 0; i < workers; ++i) { sum += counters[i].Load(MemoryOrder::Acquire); }
        auto rate = sum - prev; prev = sum;
        printf("HighPressure running: %u/60 s, ops=%llu, ops/s=%llu\n", sec, (unsigned long long)sum, (unsigned long long)rate);
    }
    Concurrency::WaitForCounter(c);
    auto total = Platform::Time::DeltaSeconds(startNs, Platform::Time::Now());
    printf("HighPressure completed in %.3fs, ops=%llu\n", total, (unsigned long long)prev);
    (void)Platform::Memory::Heap::FreeRaw(h, counters);
    (void)sch.Stop();
    return 0;
}