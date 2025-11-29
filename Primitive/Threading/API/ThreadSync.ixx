export module Prm.Threading:ThreadSync;
import Lang.Element;
import Lang.Flow;
import :Types;

export namespace Prm {
    export [[nodiscard]] Expect<ThreadHandle> ThreadCreate(ThreadFunc func, void* user) noexcept;
    export Status ThreadJoin(ThreadHandle h) noexcept;
    export void   ThreadYield() noexcept;
    export void   ThreadSleepMs(UInt32 ms) noexcept;
    export Status ThreadSetAffinityMask(ThreadHandle h, UInt64 mask) noexcept;
    export Status ThreadSetGroupAffinity(ThreadHandle h, UInt16 group, UInt64 mask) noexcept;

    export [[nodiscard]] Expect<MutexHandle>     MutexCreate() noexcept;
    export Status MutexDestroy(MutexHandle h) noexcept;
    export Status MutexLock(MutexHandle h) noexcept;
    export Status MutexUnlock(MutexHandle h) noexcept;
    export Status MutexTryLock(MutexHandle h) noexcept;

    export [[nodiscard]] Expect<SemaphoreHandle> SemaphoreCreate(UInt32 initial, UInt32 max) noexcept;
    export Status SemaphoreDestroy(SemaphoreHandle h) noexcept;
    export Status SemaphoreAcquire(SemaphoreHandle h, UInt32 timeoutMs) noexcept;
    export Status SemaphoreRelease(SemaphoreHandle h, UInt32 count) noexcept;

    export [[nodiscard]] Expect<EventHandle> EventCreate(bool manualReset, bool initialState) noexcept;
    export Status EventDestroy(EventHandle h) noexcept;
    export Status EventWait(EventHandle h, UInt32 timeoutMs) noexcept;
    export Status EventSignal(EventHandle h) noexcept;
    export Status EventReset(EventHandle h) noexcept;

    export Status WaitOnAddress(const void* address, const void* compareAddress, USize size, UInt32 timeoutMs) noexcept;
    export void   WakeByAddressSingle(void* address) noexcept;
    export void   WakeByAddressAll(void* address) noexcept;
}
