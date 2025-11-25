module;
module Prm.Threading;

import :Threading;

extern "C" __declspec(dllimport) void* GetModuleHandleA(const char* lpModuleName);
extern "C" __declspec(dllimport) void* GetProcAddress(void* hModule, const char* lpProcName);

namespace Prm {
    

    static int CallWaitOnAddress(void* Address, void* CompareAddress, size_t AddressSize, unsigned long dwMilliseconds);
    static void CallWakeByAddressSingle(void* Address);
    static void CallWakeByAddressAll(void* Address);

    // Win32 基本线程/同步 API
    extern "C" __declspec(dllimport) void* CreateThread(void* lpThreadAttributes, unsigned long dwStackSize,
        unsigned long(__stdcall* lpStartAddress)(void*), void* lpParameter, unsigned long dwCreationFlags, unsigned long* lpThreadId);
    extern "C" __declspec(dllimport) unsigned long WaitForSingleObject(void* hHandle, unsigned long dwMilliseconds);
    extern "C" __declspec(dllimport) int CloseHandle(void* hObject);
    extern "C" __declspec(dllimport) void Sleep(unsigned long ms);
    extern "C" __declspec(dllimport) int SwitchToThread(void);
    extern "C" __declspec(dllimport) unsigned long long SetThreadAffinityMask(void* hThread, unsigned long long dwThreadAffinityMask);
    struct GROUP_AFFINITY { unsigned long long Mask; unsigned short Group; unsigned short Reserved[3]; };
    extern "C" __declspec(dllimport) int SetThreadGroupAffinity(void* hThread, const GROUP_AFFINITY* GroupAffinity, GROUP_AFFINITY* PreviousGroupAffinity);

    extern "C" __declspec(dllimport) void* CreateMutexA(void* lpMutexAttributes, int bInitialOwner, const char* lpName);
    extern "C" __declspec(dllimport) int   ReleaseMutex(void* hMutex);

    extern "C" __declspec(dllimport) void* CreateSemaphoreA(void* lpSemaphoreAttributes, long lInitialCount, long lMaximumCount, const char* lpName);
    extern "C" __declspec(dllimport) int   ReleaseSemaphore(void* hSemaphore, long lReleaseCount, long* lpPreviousCount);

    extern "C" __declspec(dllimport) void* CreateEventA(void* lpEventAttributes, int bManualReset, int bInitialState, const char* lpName);
    extern "C" __declspec(dllimport) int   SetEvent(void* hEvent);
    extern "C" __declspec(dllimport) int   ResetEvent(void* hEvent);


    static constexpr unsigned long kINFINITE = 0xFFFFFFFFul;
    static constexpr unsigned long kWAIT_OBJECT_0 = 0x00000000ul;

    struct StartArgs { ThreadFunc func; void* user; };

    extern "C" __declspec(dllimport) void* GetProcessHeap(void);
    extern "C" __declspec(dllimport) void* HeapAlloc(void* hHeap, unsigned long dwFlags, size_t dwBytes);
    extern "C" __declspec(dllimport) int   HeapFree(void* hHeap, unsigned long dwFlags, void* lpMem);

    static unsigned long __stdcall ThreadStart(void* p) {
        StartArgs* args = static_cast<StartArgs*>(p);
        if (args && args->func) {
            args->func(args->user);
        }
        // 释放启动参数内存（由线程自身回收，避免泄漏）
        void* heap = GetProcessHeap();
        if (heap && args) { HeapFree(heap, 0u, args); }
        return 0u;
    }

    Expect<ThreadHandle> ThreadCreate(ThreadFunc func, void* user) noexcept {
        // 为启动参数分配一小块内存（使用堆由平台管理）；避免依赖 STL 或 new/delete。
        // VirtualAlloc/VirtualFree 可选，但在多数场景下由 CRT 堆足够；此处直接使用 CreateThread 的参数传递。
        void* heap = GetProcessHeap();
        StartArgs* args = reinterpret_cast<StartArgs*>(HeapAlloc(heap, 0u, sizeof(StartArgs)));
        if (!args) {
            return Expect<ThreadHandle>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        args->func = func;
        args->user = user;

        unsigned long tid = 0;
        void* h = CreateThread(nullptr, 0u, &ThreadStart, args, 0u, &tid);
        if (!h) {
            if (heap && args) HeapFree(heap, 0u, args);
            return Expect<ThreadHandle>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        return Expect<ThreadHandle>::Ok(ThreadHandle{h});
    }

    Status ThreadJoin(ThreadHandle h) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const unsigned long w = WaitForSingleObject(h.Get(), kINFINITE);
        // 清理线程句柄
        CloseHandle(h.Get());
        return w == kWAIT_OBJECT_0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }
    void ThreadYield() noexcept { SwitchToThread(); }
    void ThreadSleepMs(UInt32 ms) noexcept { Sleep(static_cast<unsigned long>(ms)); }
    Status ThreadSetAffinityMask(ThreadHandle h, UInt64 mask) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const unsigned long long prev = SetThreadAffinityMask(h.Get(), static_cast<unsigned long long>(mask));
        return prev ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }
    Status ThreadSetGroupAffinity(ThreadHandle h, UInt16 group, UInt64 mask) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        GROUP_AFFINITY ga{}; ga.Mask = static_cast<unsigned long long>(mask); ga.Group = group;
        const int ok = SetThreadGroupAffinity(h.Get(), &ga, nullptr);
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    // --- Mutex (opaque handle) ---
    Expect<MutexHandle> MutexCreate() noexcept {
        void* h = CreateMutexA(nullptr, 0, nullptr);
        return h ? Expect<MutexHandle>::Ok(MutexHandle{h}) : Expect<MutexHandle>::Err(Err(StatusDomain::System(), StatusCode::Failed));
    }
    Status MutexDestroy(MutexHandle h) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok = CloseHandle(h.Get());
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }
    Status MutexLock(MutexHandle h) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const unsigned long r = WaitForSingleObject(h.Get(), kINFINITE);
        return r == kWAIT_OBJECT_0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }
    Status MutexUnlock(MutexHandle h) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok = ReleaseMutex(h.Get());
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }
    Status MutexTryLock(MutexHandle h) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const unsigned long r = WaitForSingleObject(h.Get(), 0u);
        return r == kWAIT_OBJECT_0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Timeout);
    }

    // --- Semaphore (opaque handle) ---
    Expect<SemaphoreHandle> SemaphoreCreate(UInt32 initial, UInt32 max) noexcept {
        void* h = CreateSemaphoreA(nullptr, static_cast<long>(initial), static_cast<long>(max), nullptr);
        return h ? Expect<SemaphoreHandle>::Ok(SemaphoreHandle{h}) : Expect<SemaphoreHandle>::Err(Err(StatusDomain::System(), StatusCode::Failed));
    }
    Status SemaphoreDestroy(SemaphoreHandle h) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok = CloseHandle(h.Get());
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }
    Status SemaphoreAcquire(SemaphoreHandle h, UInt32 timeoutMs) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const unsigned long r = WaitForSingleObject(h.Get(), static_cast<unsigned long>(timeoutMs));
        return r == kWAIT_OBJECT_0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Timeout);
    }
    Status SemaphoreRelease(SemaphoreHandle h, UInt32 count) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok = ReleaseSemaphore(h.Get(), static_cast<long>(count), nullptr);
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    // --- Event (opaque handle) ---
    Expect<EventHandle> EventCreate(bool manualReset, bool initialState) noexcept {
        void* h = CreateEventA(nullptr, manualReset ? 1 : 0, initialState ? 1 : 0, nullptr);
        return h ? Expect<EventHandle>::Ok(EventHandle{h}) : Expect<EventHandle>::Err(Err(StatusDomain::System(), StatusCode::Failed));
    }
    Status EventDestroy(EventHandle h) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok = CloseHandle(h.Get());
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }
    Status EventWait(EventHandle h, UInt32 timeoutMs) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const unsigned long r = WaitForSingleObject(h.Get(), static_cast<unsigned long>(timeoutMs));
        return r == kWAIT_OBJECT_0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Timeout);
    }
    Status EventSignal(EventHandle h) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok = SetEvent(h.Get());
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }
    Status EventReset(EventHandle h) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok = ResetEvent(h.Get());
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Status WaitOnAddress(const void* address, const void* compareAddress, USize size, UInt32 timeoutMs) noexcept {
        void* a = const_cast<void*>(address);
        void* c = const_cast<void*>(compareAddress);
        const int r = CallWaitOnAddress(a, c, static_cast<size_t>(size), static_cast<unsigned long>(timeoutMs));
        return r ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Timeout);
    }
    void WakeByAddressSingle(void* address) noexcept { CallWakeByAddressSingle(address); }
    void WakeByAddressAll(void* address) noexcept { CallWakeByAddressAll(address); }
    static int CallWaitOnAddress(void* Address, void* CompareAddress, size_t AddressSize, unsigned long dwMilliseconds) {
        static void* mod = GetModuleHandleA("Kernel32.dll");
        static auto p = reinterpret_cast<int(*)(void*, void*, size_t, unsigned long)>(GetProcAddress(mod, "WaitOnAddress"));
        if (!p) {
            mod = GetModuleHandleA("KernelBase.dll");
            p = reinterpret_cast<int(*)(void*, void*, size_t, unsigned long)>(GetProcAddress(mod, "WaitOnAddress"));
        }
        if (!p) return 0;
        return p(Address, CompareAddress, AddressSize, dwMilliseconds);
    }
    static void CallWakeByAddressSingle(void* Address) {
        static void* mod = GetModuleHandleA("Kernel32.dll");
        static auto p = reinterpret_cast<void(*)(void*)>(GetProcAddress(mod, "WakeByAddressSingle"));
        if (!p) {
            mod = GetModuleHandleA("KernelBase.dll");
            p = reinterpret_cast<void(*)(void*)>(GetProcAddress(mod, "WakeByAddressSingle"));
        }
        if (p) p(Address);
    }
    static void CallWakeByAddressAll(void* Address) {
        static void* mod = GetModuleHandleA("Kernel32.dll");
        static auto p = reinterpret_cast<void(*)(void*)>(GetProcAddress(mod, "WakeByAddressAll"));
        if (!p) {
            mod = GetModuleHandleA("KernelBase.dll");
            p = reinterpret_cast<void(*)(void*)>(GetProcAddress(mod, "WakeByAddressAll"));
        }
        if (p) p(Address);
    }

    extern "C" void Nova_SaveContext(void* ctx);
    extern "C" void Nova_JumpContext(void* ctx);
    extern "C" void Nova_SwapContexts(void* from, void* to);

    void SaveContext(FiberContext& ctx) noexcept { Nova_SaveContext(&ctx); }
    [[noreturn]] void JumpContext(FiberContext& ctx) noexcept { Nova_JumpContext(&ctx); __assume(0); }
    void SwapContexts(FiberContext& from, FiberContext& to) noexcept { Nova_SwapContexts(&from, &to); }

#if defined(PLATFORM_FIBER_BACKEND_OS)
    extern "C" __declspec(dllimport) void* ConvertThreadToFiber(void* param);
    extern "C" __declspec(dllimport) int   ConvertFiberToThread(void);
    void EnsureThreadIsFiber(FiberContext& host) noexcept {
        if (!host.fiberHandle) { host.fiberHandle = ConvertThreadToFiber(nullptr); }
    }
    void RevertFiberToThread(FiberContext& host) noexcept {
        if (host.fiberHandle) { (void)ConvertFiberToThread(); host.fiberHandle = nullptr; }
    }
#else
    void EnsureThreadIsFiber(FiberContext&) noexcept {}
    void RevertFiberToThread(FiberContext&) noexcept {}
#endif
    bool IsOSFiberBackend() noexcept {
#if defined(PLATFORM_FIBER_BACKEND_OS)
        return true;
#else
        return false;
#endif
    }
}
