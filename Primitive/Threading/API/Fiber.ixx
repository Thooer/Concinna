export module Prm.Threading:Fiber;
import Element;
import :Types;

export namespace Prm {
    export struct FiberContext {
        void* rsp{nullptr};
        void* rip{nullptr};
        void* rbx{nullptr};
        void* rbp{nullptr};
        void* r12{nullptr};
        void* r13{nullptr};
        void* r14{nullptr};
        void* r15{nullptr};
        void* rdi{nullptr};
        void* rsi{nullptr};
        void* fiberHandle{nullptr};
        void* owner{nullptr};
        void (*entry)(void*) noexcept{nullptr};
        void* arg{nullptr};
        FiberContext* ret{nullptr};
        bool  heavy{false};
    };

    export void SaveContext(FiberContext& ctx) noexcept;
    export [[noreturn]] void JumpContext(FiberContext& ctx) noexcept;
    export void SwapContexts(FiberContext& from, FiberContext& to) noexcept;
    export void EnsureThreadIsFiber(FiberContext& host) noexcept;
    export void RevertFiberToThread(FiberContext& host) noexcept;
    export bool IsOSFiberBackend() noexcept;
}
