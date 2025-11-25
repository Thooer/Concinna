import Language;
import Platform;
import System;
import Memory;
import Concurrency;
import <cstdio>;

struct PressureCtx { Int64 endNs; Atomic<UInt64>* counter; UInt64 seed; };

static void Worker(void* p) noexcept {
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
    auto ci = System::SystemInfo::Cpu();
    UInt32 threads = ci.logicalCores ? ci.logicalCores : 1u;
    auto start = Platform::Time::Now();
    auto endNs = start + 60'000'000'000ll;
    printf("HighPressureThreads start: %u threads, 60s\n", threads);
    Platform::ThreadHandle ths[256]{};
    if (threads > 256u) threads = 256u;
    auto h = Platform::Memory::Heap::GetProcessDefault();
    auto rc = Platform::Memory::Heap::AllocRaw(h, sizeof(Atomic<UInt64>) * threads);
    if (!rc.IsOk()) return 1;
    auto* counters = static_cast<Atomic<UInt64>*>(rc.Value());
    for (UInt32 i = 0; i < threads; ++i) { new (counters + i) Atomic<UInt64>(0u); }
    
    for (UInt32 i = 0; i < threads; ++i) {
        auto rn = Platform::Memory::Heap::AllocRaw(h, sizeof(PressureCtx));
        if (!rn.IsOk()) continue;
        auto* ctx = static_cast<PressureCtx*>(rn.Value());
        ctx->endNs = endNs;
        ctx->counter = counters + i;
        ctx->seed = static_cast<UInt64>(i + 1u);
        auto t = Platform::ThreadCreate(&Worker, ctx);
        if (!t.IsOk()) { (void)Platform::Memory::Heap::FreeRaw(h, ctx); continue; }
        ths[i] = t.Value();
    }
    UInt64 prev = 0u;
    for (UInt32 sec = 1; sec <= 60; ++sec) {
        Platform::ThreadSleepMs(1000);
        UInt64 sum = 0u;
        for (UInt32 i = 0; i < threads; ++i) { sum += counters[i].Load(MemoryOrder::Acquire); }
        auto rate = sum - prev; prev = sum;
        printf("HighPressureThreads running: %u/%u s, ops=%llu, ops/s=%llu\n", sec, 60u, (unsigned long long)sum, (unsigned long long)rate);
    }
    for (UInt32 i = 0; i < threads; ++i) { (void)Platform::ThreadJoin(ths[i]); }
    auto total = Platform::Time::DeltaSeconds(start, Platform::Time::Now());
    printf("HighPressureThreads completed in %.3fs, ops=%llu\n", total, (unsigned long long)prev);
    (void)Platform::Memory::Heap::FreeRaw(h, counters);
    return 0;
}