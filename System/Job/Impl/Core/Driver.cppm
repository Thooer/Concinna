module System.Job;
extern "C" __declspec(dllimport) int GetQueuedCompletionStatus(void* h, unsigned long* bytes, unsigned long* key, void** ov, unsigned long ms);
extern "C" __declspec(dllimport) void* CreateIoCompletionPort(void* fileHandle, void* existingPort, unsigned long key, unsigned long concurrency);
import Language;
import Memory;
import Prm.Time;
import Prm.Threading:ThreadSync;
import Prm.Threading:Fiber;
import Prm.Ownership:Memory;
import System.Job:Driver;
import Cap.Concurrency:Fiber;
import System.Job:Scheduler;

namespace Sys {
    struct TimerItem { Prm::TimePoint start; UInt64 waitNs; Cap::Fiber* fb; void(*cb)(void*) noexcept; void* ctx; };
    static struct { TimerItem* items{nullptr}; USize cap{0}; USize count{0}; } gTimerStore;
    struct EventItem { Prm::EventHandle h; Cap::Fiber* fb; };
    static struct { EventItem* items{nullptr}; USize cap{0}; USize count{0}; } gEventStore;
    static struct { void* port{nullptr}; struct MapItem{ void* ov; Cap::Fiber* fb; }; MapItem* maps{nullptr}; USize cap{0}; USize count{0}; } gIocp;

    Driver gDriver{};
    IDriver* gDriverApi = &gDriver;
    struct TimerDriver : ITimerDriver {
        UInt32 NextTimeoutMs() noexcept override {
            if (!gTimerStore.items || gTimerStore.count == 0) return 1u;
            auto now = Prm::Now();
            auto left = gTimerStore.items[0].waitNs;
            auto elapsed = Prm::Delta(gTimerStore.items[0].start, now);
            UInt64 rem = (elapsed >= left) ? 0ull : (left - elapsed);
            return static_cast<UInt32>((rem + 999'999ull) / 1'000'000ull);
        }
        bool AddTimer(Cap::Fiber* fb, UInt32 delayMs) noexcept override { return gDriver.AddTimer(fb, delayMs); }
        bool AddTimeout(void(*cb)(void*) noexcept, void* ctx, UInt32 delayMs) noexcept override { return gDriver.AddTimeout(cb, ctx, delayMs); }
    };
    struct IoDriver : IIODriver {
        bool AttachIocp() noexcept override { return gDriver.AttachIocp(); }
        bool RegisterIocpHandle(void* h) noexcept override { return gDriver.RegisterIocpHandle(h); }
        bool AwaitIocp(void* ov, Cap::Fiber* fb) noexcept override { return gDriver.AwaitIocp(ov, fb); }
        void Poll() noexcept override {
            if (!gIocp.port) return;
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
        [[nodiscard]] UInt32 PendingCount() noexcept override { return static_cast<UInt32>(gIocp.count); }
    };
    struct SignalDriver : ISignalDriver {
        bool AddEvent(Prm::EventHandle h, Cap::Fiber* fb) noexcept override { return gDriver.AddEvent(h, fb); }
    };
    static TimerDriver sTimer{};
    static IoDriver sIo{};
    static SignalDriver sSignal{};
    ITimerDriver* gTimerApi = &sTimer;
    IIODriver* gIoApi = &sIo;
    ISignalDriver* gSignalApi = &sSignal;

    bool Driver::Init() noexcept {
        if (gTimerStore.items) return true;
        auto h = Prm::Heap::GetProcessDefault();
        auto rn = Prm::Heap::AllocRaw(h, sizeof(TimerItem) * 8192);
        if (!rn.IsOk()) return false;
        gTimerStore.items = static_cast<TimerItem*>(rn.Value());
        gTimerStore.cap = 8192; gTimerStore.count = 0;
        auto re = Prm::Heap::AllocRaw(h, sizeof(EventItem) * 4096);
        if (!re.IsOk()) return false;
        gEventStore.items = static_cast<EventItem*>(re.Value());
        gEventStore.cap = 4096; gEventStore.count = 0;
        auto rm = Prm::Heap::AllocRaw(h, sizeof(gIocp.maps[0]) * 8192);
        if (!rm.IsOk()) return false;
        gIocp.maps = static_cast<decltype(gIocp.maps)>(rm.Value());
        gIocp.cap = 8192; gIocp.count = 0;
        (void)AttachIocp();
        return true;
    }

    void Driver::Shutdown() noexcept {
        auto h = Prm::Heap::GetProcessDefault();
        if (gTimerStore.items) { (void)Prm::Heap::FreeRaw(h, gTimerStore.items); gTimerStore.items = nullptr; gTimerStore.cap = 0; gTimerStore.count = 0; }
        if (gEventStore.items) { (void)Prm::Heap::FreeRaw(h, gEventStore.items); gEventStore.items = nullptr; gEventStore.cap = 0; gEventStore.count = 0; }
        if (gIocp.maps) { (void)Prm::Heap::FreeRaw(h, gIocp.maps); gIocp.maps = nullptr; gIocp.cap = 0; gIocp.count = 0; }
    }

    bool Driver::AddTimer(Cap::Fiber* fb, UInt32 delayMs) noexcept {
        if (!fb) return false;
        if (!gTimerStore.items || gTimerStore.count >= static_cast<USize>(gTimerStore.cap)) return false;
        TimerItem it{}; it.start = Prm::Now(); it.waitNs = static_cast<UInt64>(delayMs) * 1'000'000ull; it.fb = fb; it.cb = nullptr; it.ctx = nullptr;
        USize i = gTimerStore.count++;
        gTimerStore.items[i] = it;
        while (i > 0) {
            auto p = (i - 1) >> 1;
            auto left = gTimerStore.items[i].waitNs - Prm::Delta(gTimerStore.items[i].start, Prm::Now());
            auto pr   = gTimerStore.items[p].waitNs - Prm::Delta(gTimerStore.items[p].start, Prm::Now());
            if (left < pr) { auto tmp = gTimerStore.items[p]; gTimerStore.items[p] = gTimerStore.items[i]; gTimerStore.items[i] = tmp; i = p; } else break;
        }
        return true;
    }

    bool Driver::AddTimeout(void(*cb)(void*) noexcept, void* ctx, UInt32 delayMs) noexcept {
        if (!cb) return false;
        if (!gTimerStore.items || gTimerStore.count >= static_cast<USize>(gTimerStore.cap)) return false;
        TimerItem it{}; it.start = Prm::Now(); it.waitNs = static_cast<UInt64>(delayMs) * 1'000'000ull; it.fb = nullptr; it.cb = cb; it.ctx = ctx;
        USize i = gTimerStore.count++;
        gTimerStore.items[i] = it;
        while (i > 0) {
            auto p = (i - 1) >> 1;
            auto left = gTimerStore.items[i].waitNs - Prm::Delta(gTimerStore.items[i].start, Prm::Now());
            auto pr   = gTimerStore.items[p].waitNs - Prm::Delta(gTimerStore.items[p].start, Prm::Now());
            if (left < pr) { auto tmp = gTimerStore.items[p]; gTimerStore.items[p] = gTimerStore.items[i]; gTimerStore.items[i] = tmp; i = p; } else break;
        }
        return true;
    }

    bool Driver::AddEvent(Prm::EventHandle h, Cap::Fiber* fb) noexcept {
        if (!h.Get() || !fb) return false;
        if (!gEventStore.items || gEventStore.count >= static_cast<USize>(gEventStore.cap)) return false;
        EventItem it{}; it.h = h; it.fb = fb;
        gEventStore.items[gEventStore.count++] = it;
        return true;
    }

    void Driver::WakeUp() noexcept {}

    UInt32 Driver::NextTimeoutMs() noexcept { return gTimerApi->NextTimeoutMs(); }

    void Driver::Poll() noexcept { gIoApi->Poll(); }

    void SleepMs(UInt32 ms) noexcept {
        auto* cf = Scheduler::CurrentFiber();
        if (!cf) { Prm::ThreadSleepMs(ms); return; }
        (void)gDriverApi->AddTimer(cf, ms);
        cf->state = Cap::FiberState::Waiting;
        Prm::SwapContexts(cf->ctx, *cf->retCtx);
        cf->state = Cap::FiberState::Running;
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

    bool Driver::AwaitIocp(void* ov, Cap::Fiber* fb) noexcept {
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
