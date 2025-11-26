module Prm.Threading;

import Prm.Threading;
import Element;

extern "C" void Nova_FiberExitNotify(void* fiberPtr) noexcept;

extern "C" __declspec(dllimport) void* ConvertThreadToFiber(void* param);
extern "C" __declspec(dllimport) void* CreateFiber(size_t dwStackSize, void (__stdcall *lpStartAddress)(void*), void* lpParameter);
extern "C" __declspec(dllimport) void  SwitchToFiber(void* fiber);
extern "C" __declspec(dllimport) void  DeleteFiber(void* fiber);

static void __stdcall OsFiberStart(void* p) {
    auto* ctx = static_cast<Prm::FiberContext*>(p);
    auto* fn = ctx->entry;
    auto* arg = ctx->arg;
    if (fn) { fn(arg); }
    Nova_FiberExitNotify(ctx->owner);
}

extern "C" void Nova_SaveContext(void* ctx) { (void)ctx; }
extern "C" void Nova_JumpContext(void* ctx) { auto* c = static_cast<Prm::FiberContext*>(ctx); SwitchToFiber(c->fiberHandle); __assume(0); }
extern "C" void Nova_SwapContexts(void* from, void* to) {
    auto* f = static_cast<Prm::FiberContext*>(from);
    auto* t = static_cast<Prm::FiberContext*>(to);
    if (!f->fiberHandle) {
        f->fiberHandle = ConvertThreadToFiber(nullptr);
    }
    if (!t->fiberHandle) {
        t->fiberHandle = CreateFiber(0, &OsFiberStart, t);
    }
    SwitchToFiber(t->fiberHandle);
}
extern "C" void Nova_FiberEnter() noexcept {}
extern "C" void Nova_FiberExit() noexcept {}
