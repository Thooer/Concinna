module;
module Prm.Threading;

import :Threading;
import Prm.IO;

extern "C" void Nova_FiberExitNotify(void* fiberPtr) noexcept;

extern "C" __declspec(dllimport) void* ConvertThreadToFiber(void* param);
extern "C" __declspec(dllimport) void* CreateFiber(size_t dwStackSize, void (__stdcall *lpStartAddress)(void*), void* lpParameter);
extern "C" __declspec(dllimport) void  SwitchToFiber(void* fiber);
extern "C" __declspec(dllimport) void  DeleteFiber(void* fiber);

static void __stdcall OsFiberStart(void* p) {
    auto* ctx = static_cast<FiberContext*>(p);
    auto* fn = ctx->entry;
    auto* arg = ctx->arg;
    {
        auto h = File::Stdout();
        const char* s = "OsStart\n";
        (void)File::Write(h, reinterpret_cast<const Byte*>(s), 8);
    }
    if (fn) { fn(arg); }
    Nova_FiberExitNotify(ctx->owner);
}

extern "C" void Nova_SaveContext(void* ctx) { (void)ctx; }
extern "C" void Nova_JumpContext(void* ctx) { auto* c = static_cast<FiberContext*>(ctx); SwitchToFiber(c->fiberHandle); __assume(0); }
extern "C" void Nova_SwapContexts(void* from, void* to) {
    auto* f = static_cast<FiberContext*>(from);
    auto* t = static_cast<FiberContext*>(to);
    if (!f->fiberHandle) {
        {
            auto h = File::Stdout(); const char* s = "Conv\n"; (void)File::Write(h, reinterpret_cast<const Byte*>(s), 5);
        }
        f->fiberHandle = ConvertThreadToFiber(nullptr);
    }
    if (!t->fiberHandle) {
        {
            auto h = File::Stdout(); const char* s = "Create\n"; (void)File::Write(h, reinterpret_cast<const Byte*>(s), 7);
        }
        t->fiberHandle = CreateFiber(0, &OsFiberStart, t);
    }
    {
        auto h = File::Stdout(); const char* s = "Switch\n"; (void)File::Write(h, reinterpret_cast<const Byte*>(s), 7);
    }
    SwitchToFiber(t->fiberHandle);
}
extern "C" void Nova_FiberEnter() noexcept {}
extern "C" void Nova_FiberExit() noexcept {}
