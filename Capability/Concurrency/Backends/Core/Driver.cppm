module Concurrency;
extern "C" __declspec(dllimport) int GetQueuedCompletionStatus(void* h, unsigned long* bytes, unsigned long* key, void** ov, unsigned long ms);
extern "C" __declspec(dllimport) void* CreateIoCompletionPort(void* fileHandle, void* existingPort, unsigned long key, unsigned long concurrency);
import Language;
import Memory;
import Platform;
import :Driver;
import :Fiber;
import :Scheduler;

namespace Concurrency {
    struct TimerItem { Platform::Time::TimePoint start; UInt64 waitNs; Fiber* fb; void(*cb)(void*) noexcept; void* ctx; };
    static struct { TimerItem* items{nullptr}; USize cap{0}; USize count{0}; } gTimerStore;
    struct EventItem { Platform::EventHandle h; Fiber* fb; };
    static struct { EventItem* items{nullptr}; USize cap{0}; USize count{0}; } gEventStore;
    static struct { void* port{nullptr}; struct MapItem{ void* ov; Fiber* fb; }; MapItem* maps{nullptr}; USize cap{0}; USize count{0}; } gIocp;

    Driver gDriver{};
    IDriver* gDriverApi = &gDriver;

    bool Driver::Init() noexcept {
        if (gTimerStore.items) return true;
        auto h = Platform::Memory::Heap::GetProcessDefault();
        auto rn = Platform::Memory::Heap::AllocRaw(h, sizeof(TimerItem) * 8192);
        if (!rn.IsOk()) return false;
        gTimerStore.items = static_cast<TimerItem*>(rn.Value());
        gTimerStore.cap = 8192; gTimerStore.count = 0;
        auto re = Platform::Memory::Heap::AllocRaw(h, sizeof(EventItem) * 4096);
        if (!re.IsOk()) return false;
        gEventStore.items = static_cast<EventItem*>(re.Value());
        gEventStore.cap = 4096; gEventStore.count = 0;
        auto rm = Platform::Memory::Heap::AllocRaw(h, sizeof(gIocp.maps[0]) * 8192);
        if (!rm.IsOk()) return false;
        gIocp.maps = static_cast<decltype(gIocp.maps)>(rm.Value());
        gIocp.cap = 8192; gIocp.count = 0;
        (void)AttachIocp();
        return true;
    }

    void Driver::Shutdown() noexcept {
        auto h = Platform::Memory::Heap::GetProcessDefault();
        if (gTimerStore.items) { (void)Platform::Memory::Heap::FreeRaw(h, gTimerStore.items); gTimerStore.items = nullptr; gTimerStore.cap = 0; gTimerStore.count = 0; }
        if (gEventStore.items) { (void)Platform::Memory::Heap::FreeRaw(h, gEventStore.items); gEventStore.items = nullptr; gEventStore.cap = 0; gEventStore.count = 0; }
        if (gIocp.maps) { (void)Platform::Memory::Heap::FreeRaw(h, gIocp.maps); gIocp.maps = nullptr; gIocp.cap = 0; gIocp.count = 0; }
    }

    bool Driver::AddTimer(Fiber* fb, UInt32 delayMs) noexcept {
        if (!fb) return false;
        if (!gTimerStore.items || gTimerStore.count >= static_cast<USize>(gTimerStore.cap)) return false;
        TimerItem it{}; it.start = Platform::Time::Now(); it.waitNs = static_cast<UInt64>(delayMs) * 1'000'000ull; it.fb = fb; it.cb = nullptr; it.ctx = nullptr;
        // push to heap bottom
        USize i = gTimerStore.count++;
        gTimerStore.items[i] = it;
        // sift up
        while (i > 0) {
            auto p = (i - 1) >> 1;
            auto left = gTimerStore.items[i].waitNs - Platform::Time::Delta(gTimerStore.items[i].start, Platform::Time::Now());
            auto pr   = gTimerStore.items[p].waitNs - Platform::Time::Delta(gTimerStore.items[p].start, Platform::Time::Now());
            if (left < pr) { auto tmp = gTimerStore.items[p]; gTimerStore.items[p] = gTimerStore.items[i]; gTimerStore.items[i] = tmp; i = p; } else break;
        }
        return true;
    }

    bool Driver::AddTimeout(void(*cb)(void*) noexcept, void* ctx, UInt32 delayMs) noexcept {
        if (!cb) return false;
        if (!gTimerStore.items || gTimerStore.count >= static_cast<USize>(gTimerStore.cap)) return false;
        TimerItem it{}; it.start = Platform::Time::Now(); it.waitNs = static_cast<UInt64>(delayMs) * 1'000'000ull; it.fb = nullptr; it.cb = cb; it.ctx = ctx;
        USize i = gTimerStore.count++;
        gTimerStore.items[i] = it;
        while (i > 0) {
            auto p = (i - 1) >> 1;
            auto left = gTimerStore.items[i].waitNs - Platform::Time::Delta(gTimerStore.items[i].start, Platform::Time::Now());
            auto pr   = gTimerStore.items[p].waitNs - Platform::Time::Delta(gTimerStore.items[p].start, Platform::Time::Now());
            if (left < pr) { auto tmp = gTimerStore.items[p]; gTimerStore.items[p] = gTimerStore.items[i]; gTimerStore.items[i] = tmp; i = p; } else break;
        }
        return true;
    }

    bool Driver::AddEvent(Platform::EventHandle h, Fiber* fb) noexcept {
        if (!h.Get() || !fb) return false;
        if (!gEventStore.items || gEventStore.count >= static_cast<USize>(gEventStore.cap)) return false;
        EventItem it{}; it.h = h; it.fb = fb;
        gEventStore.items[gEventStore.count++] = it;
        return true;
    }

    void Driver::WakeUp() noexcept {}

    UInt32 Driver::NextTimeoutMs() noexcept {
        if (!gTimerStore.items || gTimerStore.count == 0) return 1u;
        auto now = Platform::Time::Now();
        auto left = gTimerStore.items[0].waitNs;
        auto elapsed = Platform::Time::Delta(gTimerStore.items[0].start, now);
        UInt64 rem = (elapsed >= left) ? 0ull : (left - elapsed);
        return static_cast<UInt32>((rem + 999'999ull) / 1'000'000ull);
    }

    void Driver::Poll() noexcept {
        if (gIocp.port) {
            unsigned long bytes = 0; unsigned long key = 0; void* ov = nullptr;
            int ok = GetQueuedCompletionStatus(gIocp.port, &bytes, &key, &ov, 0u);
            if (ok && ov) {
                for (USize i = 0; i < gIocp.count; ++i) {
                    if (gIocp.maps[i].ov == ov) {
                        auto* fb = gIocp.maps[i].fb;
                        gIocp.maps[i] = gIocp.maps[gIocp.count - 1];
                        --gIocp.count;
                        if (fb) Scheduler::Instance().ResumeFiber(fb);
                        break;
                    }
                }
            }
        }
        if (gEventStore.items && gEventStore.count > 0) {
            USize i = 0;
            while (i < gEventStore.count) {
                auto r = Platform::EventWait(gEventStore.items[i].h, 0);
                if (r.Ok()) {
                    auto* fb = gEventStore.items[i].fb;
                    gEventStore.items[i] = gEventStore.items[gEventStore.count - 1];
                    --gEventStore.count;
                    if (fb) Scheduler::Instance().ResumeFiber(fb);
                    continue;
                }
                ++i;
            }
        }

        if (!gTimerStore.items || gTimerStore.count == 0) return;
        auto now = Platform::Time::Now();
        while (gTimerStore.count > 0) {
            auto left = gTimerStore.items[0].waitNs;
            auto elapsed = Platform::Time::Delta(gTimerStore.items[0].start, now);
            if (elapsed >= left) {
                auto* fb = gTimerStore.items[0].fb;
                auto* cb = gTimerStore.items[0].cb;
                void* ctx = gTimerStore.items[0].ctx;
                gTimerStore.items[0] = gTimerStore.items[gTimerStore.count - 1];
                --gTimerStore.count;
                // sift down
                USize i = 0;
                while (true) {
                    auto l = (i << 1) + 1;
                    auto r = l + 1;
                    if (l >= gTimerStore.count) break;
                    auto li = gTimerStore.items[l].waitNs - Platform::Time::Delta(gTimerStore.items[l].start, now);
                    USize mi = l;
                    if (r < gTimerStore.count) {
                        auto ri = gTimerStore.items[r].waitNs - Platform::Time::Delta(gTimerStore.items[r].start, now);
                        if (ri < li) { mi = r; li = ri; }
                    }
                    auto ii = gTimerStore.items[i].waitNs - Platform::Time::Delta(gTimerStore.items[i].start, now);
                    if (li < ii) { auto tmp = gTimerStore.items[mi]; gTimerStore.items[mi] = gTimerStore.items[i]; gTimerStore.items[i] = tmp; i = mi; } else break;
                }
                if (fb) Scheduler::Instance().ResumeFiber(fb);
                else if (cb) cb(ctx);
                continue;
            }
            break;
        }
    }

    void SleepMs(UInt32 ms) noexcept {
        auto* cf = Scheduler::CurrentFiber();
        if (!cf) { Platform::ThreadSleepMs(ms); return; }
        (void)gDriverApi->AddTimer(cf, ms);
        cf->state = FiberState::Waiting;
        Platform::SwapContexts(cf->ctx, *cf->retCtx);
        cf->state = FiberState::Running;
    }

    bool Driver::AttachIocp() noexcept {
        if (gIocp.port) return true;
        void* p = CreateIoCompletionPort(reinterpret_cast<void*>(~0ull), nullptr, 0u, 0u);
        if (!p) return false;
        gIocp.port = p; return true;
    }

    bool Driver::RegisterIocpHandle(void* h) noexcept {
        if (!AttachIocp()) return false;
        void* p = CreateIoCompletionPort(h, gIocp.port, 0u, 0u);
        return p != nullptr;
    }

    bool Driver::AwaitIocp(void* ov, Fiber* fb) noexcept {
        if (!gIocp.port || !ov || !fb) return false;
        if (gIocp.count >= static_cast<USize>(gIocp.cap)) return false;
        gIocp.maps[gIocp.count++] = { ov, fb };
        return true;
    }

    UInt32 Driver::PendingCount() noexcept {
        UInt32 cnt = 0u;
        cnt += static_cast<UInt32>(gTimerStore.count);
        cnt += static_cast<UInt32>(gEventStore.count);
        cnt += static_cast<UInt32>(gIocp.count);
        return cnt;
    }
}