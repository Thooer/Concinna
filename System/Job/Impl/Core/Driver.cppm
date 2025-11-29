module Sys;
extern "C" __declspec(dllimport) int GetQueuedCompletionStatus(void* h, unsigned long* bytes, unsigned long* key, void** ov, unsigned long ms);
extern "C" __declspec(dllimport) void* CreateIoCompletionPort(void* fileHandle, void* existingPort, unsigned long key, unsigned long concurrency);
extern "C" __declspec(dllimport) unsigned long long GetTickCount64();
import Lang;
import Prm.Time;
import Prm.Ownership;
import :Driver;
import :Scheduler;
import Cap.Concurrency;

namespace Sys {
    struct TimerItem { UInt64 startMs; UInt64 waitMs; Cap::Fiber* fb; void(*cb)(void*) noexcept; void* ctx; };
    static struct { TimerItem* items{nullptr}; USize cap{0}; USize count{0}; } gTimerStore;
    struct EventItem { void* h; Cap::Fiber* fb; };
    static struct { EventItem* items{nullptr}; USize cap{0}; USize count{0}; } gEventStore;
    static struct { void* port{nullptr}; struct MapItem{ void* ov; Cap::Fiber* fb; }; MapItem* maps{nullptr}; USize cap{0}; USize count{0}; } gIocp;

    Driver gDriver{};
    IDriver* gDriverApi = &gDriver;
    struct TimerDriver : ITimerDriver {
        UInt32 NextTimeoutMs() noexcept override {
            if (!gTimerStore.items || gTimerStore.count == 0) return 1u;
            auto now = GetTickCount64();
            auto left = gTimerStore.items[0].waitMs;
            auto elapsed = now - gTimerStore.items[0].startMs;
            UInt64 rem = (elapsed >= left) ? 0ull : (left - elapsed);
            return static_cast<UInt32>(rem);
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
        bool AddEvent(void* h, Cap::Fiber* fb) noexcept override { return gDriver.AddEvent(h, fb); }
    };
    static TimerDriver sTimer{};
    static IoDriver sIo{};
    static SignalDriver sSignal{};
    ITimerDriver* gTimerApi = &sTimer;
    IIODriver* gIoApi = &sIo;
    ISignalDriver* gSignalApi = &sSignal;

    bool Driver::Init() noexcept {
        if (gTimerStore.items) return true;
        auto* ti = static_cast<TimerItem*>(::operator new(sizeof(TimerItem) * 8192));
        if (!ti) return false;
        gTimerStore.items = ti;
        gTimerStore.cap = 8192; gTimerStore.count = 0;
        auto* ei = static_cast<EventItem*>(::operator new(sizeof(EventItem) * 4096));
        if (!ei) return false;
        gEventStore.items = ei;
        gEventStore.cap = 4096; gEventStore.count = 0;
        auto* mp = static_cast<decltype(gIocp.maps)>(::operator new(sizeof(gIocp.maps[0]) * 8192));
        if (!mp) return false;
        gIocp.maps = mp;
        gIocp.cap = 8192; gIocp.count = 0;
        (void)AttachIocp();
        return true;
    }

    void Driver::Shutdown() noexcept {
        if (gTimerStore.items) { ::operator delete(gTimerStore.items); gTimerStore.items = nullptr; gTimerStore.cap = 0; gTimerStore.count = 0; }
        if (gEventStore.items) { ::operator delete(gEventStore.items); gEventStore.items = nullptr; gEventStore.cap = 0; gEventStore.count = 0; }
        if (gIocp.maps) { ::operator delete(gIocp.maps); gIocp.maps = nullptr; gIocp.cap = 0; gIocp.count = 0; }
    }

    bool Driver::AddTimer(Cap::Fiber* fb, UInt32 delayMs) noexcept {
        if (!fb) return false;
        if (!gTimerStore.items || gTimerStore.count >= static_cast<USize>(gTimerStore.cap)) return false;
        TimerItem it{}; it.startMs = GetTickCount64(); it.waitMs = static_cast<UInt64>(delayMs); it.fb = fb; it.cb = nullptr; it.ctx = nullptr;
        USize i = gTimerStore.count++;
        gTimerStore.items[i] = it;
        while (i > 0) {
            auto p = (i - 1) >> 1;
            auto left = gTimerStore.items[i].waitMs - (GetTickCount64() - gTimerStore.items[i].startMs);
            auto pr   = gTimerStore.items[p].waitMs - (GetTickCount64() - gTimerStore.items[p].startMs);
            if (left < pr) { auto tmp = gTimerStore.items[p]; gTimerStore.items[p] = gTimerStore.items[i]; gTimerStore.items[i] = tmp; i = p; } else break;
        }
        return true;
    }

    bool Driver::AddTimeout(void(*cb)(void*) noexcept, void* ctx, UInt32 delayMs) noexcept {
        if (!cb) return false;
        if (!gTimerStore.items || gTimerStore.count >= static_cast<USize>(gTimerStore.cap)) return false;
        TimerItem it{}; it.startMs = GetTickCount64(); it.waitMs = static_cast<UInt64>(delayMs); it.fb = nullptr; it.cb = cb; it.ctx = ctx;
        USize i = gTimerStore.count++;
        gTimerStore.items[i] = it;
        while (i > 0) {
            auto p = (i - 1) >> 1;
            auto left = gTimerStore.items[i].waitMs - (GetTickCount64() - gTimerStore.items[i].startMs);
            auto pr   = gTimerStore.items[p].waitMs - (GetTickCount64() - gTimerStore.items[p].startMs);
            if (left < pr) { auto tmp = gTimerStore.items[p]; gTimerStore.items[p] = gTimerStore.items[i]; gTimerStore.items[i] = tmp; i = p; } else break;
        }
        return true;
    }

    bool Driver::AddEvent(void* h, Cap::Fiber* fb) noexcept {
        if (!h || !fb) return false;
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
        if (!cf) { return; }
        (void)gDriverApi->AddTimer(cf, ms);
        cf->state = Cap::FiberState::Waiting;
        cf->StartSwitch(cf->retCtx);
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
