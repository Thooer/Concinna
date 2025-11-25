// Platform.Threading — 线程与同步原语（自由函数 + 不透明句柄）
module;
export module Platform:Threading;

import Prm;

export namespace Platform {
    

    struct ThreadHandleTag;
    struct MutexHandleTag;
    struct SemaphoreHandleTag;
    struct EventHandleTag;

    using ThreadFunc   = void(*)(void*) noexcept;
    export using ThreadHandle = StrongAlias<void*, ThreadHandleTag>;
    export [[nodiscard]] Expect<ThreadHandle> ThreadCreate(ThreadFunc func, void* user) noexcept;
    export Status ThreadJoin(ThreadHandle h) noexcept;
    export void ThreadYield() noexcept;
    export void ThreadSleepMs(UInt32 ms) noexcept;
    export Status ThreadSetAffinityMask(ThreadHandle h, UInt64 mask) noexcept;
    export Status ThreadSetGroupAffinity(ThreadHandle h, UInt16 group, UInt64 mask) noexcept;

    export using MutexHandle = StrongAlias<void*, MutexHandleTag>;
    export [[nodiscard]] Expect<MutexHandle> MutexCreate() noexcept;
    export Status MutexDestroy(MutexHandle h) noexcept;
    export Status MutexLock(MutexHandle h) noexcept;
    export Status MutexUnlock(MutexHandle h) noexcept;
    export Status MutexTryLock(MutexHandle h) noexcept;

    export using SemaphoreHandle = StrongAlias<void*, SemaphoreHandleTag>;
    export [[nodiscard]] Expect<SemaphoreHandle> SemaphoreCreate(UInt32 initial, UInt32 max) noexcept;
    export Status SemaphoreDestroy(SemaphoreHandle h) noexcept;
    export Status SemaphoreAcquire(SemaphoreHandle h, UInt32 timeoutMs) noexcept;
    export Status SemaphoreRelease(SemaphoreHandle h, UInt32 count) noexcept;

    export using EventHandle = StrongAlias<void*, EventHandleTag>;
    export [[nodiscard]] Expect<EventHandle> EventCreate(bool manualReset, bool initialState) noexcept;
    export Status EventDestroy(EventHandle h) noexcept;
    export Status EventWait(EventHandle h, UInt32 timeoutMs) noexcept;
    export Status EventSignal(EventHandle h) noexcept;
    export Status EventReset(EventHandle h) noexcept;

    export Status WaitOnAddress(const void* address, const void* compareAddress, USize size, UInt32 timeoutMs) noexcept;
    export void WakeByAddressSingle(void* address) noexcept;
    export void WakeByAddressAll(void* address) noexcept;

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
