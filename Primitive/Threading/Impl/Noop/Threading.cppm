module;
module Platform;

import Prm
import :Threading;

namespace Platform {
    

    Expect<ThreadHandle> ThreadCreate(ThreadFunc, void*) noexcept {
        return Expect<ThreadHandle>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    Status ThreadJoin(ThreadHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    void   ThreadYield() noexcept {}
    void   ThreadSleepMs(UInt32) noexcept {}

    Expect<MutexHandle> MutexCreate() noexcept { return Expect<MutexHandle>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
    Status MutexDestroy(MutexHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status MutexLock(MutexHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status MutexUnlock(MutexHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status MutexTryLock(MutexHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }

    Expect<SemaphoreHandle> SemaphoreCreate(UInt32, UInt32) noexcept { return Expect<SemaphoreHandle>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
    Status SemaphoreDestroy(SemaphoreHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status SemaphoreAcquire(SemaphoreHandle, UInt32) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status SemaphoreRelease(SemaphoreHandle, UInt32) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }

    Expect<EventHandle> EventCreate(bool, bool) noexcept { return Expect<EventHandle>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
    Status EventDestroy(EventHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status EventWait(EventHandle, UInt32) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status EventSignal(EventHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status EventReset(EventHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }

    Status WaitOnAddress(const void*, const void*, USize, UInt32) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    void WakeByAddressSingle(void*) noexcept {}
    void WakeByAddressAll(void*) noexcept {}
}