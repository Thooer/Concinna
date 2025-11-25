module;

module Platform;

import Prm
import :Memory;


using namespace Platform;
 

    // Win32 虚拟内存 API（避免 windows.h）
    extern "C" __declspec(dllimport) void* VirtualAlloc(void* lpAddress, UInt64 dwSize, unsigned long flAllocationType, unsigned long flProtect);
    extern "C" __declspec(dllimport) int   VirtualFree(void* lpAddress, unsigned long dwSize, unsigned long dwFreeType);
    extern "C" __declspec(dllimport) int   VirtualProtect(void* lpAddress, UInt64 dwSize, unsigned long flNewProtect, unsigned long* lpflOldProtect);
    extern "C" __declspec(dllimport) void  GetSystemInfo(void* lpSystemInfo);
    extern "C" __declspec(dllimport) UInt64 GetLargePageMinimum();
    extern "C" __declspec(dllimport) void* GetCurrentProcess();
    extern "C" __declspec(dllimport) void* VirtualAllocExNuma(void* hProcess, void* lpAddress, UInt64 dwSize, unsigned long flAllocationType, unsigned long flProtect, unsigned long nndPreferred);
    extern "C" __declspec(dllimport) unsigned long GetLastError();

    // Win32 进程堆 API
    extern "C" __declspec(dllimport) void* GetProcessHeap();
    extern "C" __declspec(dllimport) void* HeapAlloc(void* hHeap, unsigned long dwFlags, UInt64 dwBytes);
    extern "C" __declspec(dllimport) void* HeapReAlloc(void* hHeap, unsigned long dwFlags, void* lpMem, UInt64 dwBytes);
    extern "C" __declspec(dllimport) int   HeapFree(void* hHeap, unsigned long dwFlags, void* lpMem);

    struct SYSTEM_INFO_MIN {
        unsigned short wProcessorArchitecture;
        unsigned short wReserved;
        unsigned int   dwPageSize;
        void*          lpMinimumApplicationAddress;
        void*          lpMaximumApplicationAddress;
        void*          dwActiveProcessorMask;
        unsigned int   dwNumberOfProcessors;
        unsigned int   dwProcessorType;
        unsigned int   dwAllocationGranularity;
        unsigned short wProcessorLevel;
        unsigned short wProcessorRevision;
    };

    // 常量（必要子集）
    static constexpr unsigned long MEM_COMMIT   = 0x00001000ul;
    static constexpr unsigned long MEM_RESERVE  = 0x00002000ul;
    static constexpr unsigned long MEM_DECOMMIT = 0x00004000ul;
    static constexpr unsigned long MEM_RELEASE  = 0x00008000ul;
    static constexpr unsigned long MEM_LARGE_PAGES = 0x20000000ul;
    static constexpr unsigned long PAGE_NOACCESS          = 0x00000001ul;
    static constexpr unsigned long PAGE_READONLY          = 0x00000002ul;
    static constexpr unsigned long PAGE_READWRITE         = 0x00000004ul;
    static constexpr unsigned long PAGE_EXECUTE_READ      = 0x00000020ul;
    static constexpr unsigned long PAGE_EXECUTE_READWRITE = 0x00000040ul;

    static unsigned long ToWinProtect(Memory::PageProtection p) noexcept {
        switch (p) {
            case Memory::PageProtection::NoAccess:          return PAGE_NOACCESS;
            case Memory::PageProtection::ReadOnly:          return PAGE_READONLY;
            case Memory::PageProtection::ReadWrite:         return PAGE_READWRITE;
            case Memory::PageProtection::ExecuteRead:       return PAGE_EXECUTE_READ;
            case Memory::PageProtection::ExecuteReadWrite:  return PAGE_EXECUTE_READWRITE;
            default:                                        return PAGE_READWRITE;
        }
    }

    [[nodiscard]] static USize QueryPageSize() noexcept {
        SYSTEM_INFO_MIN si{};
        GetSystemInfo(&si);
        return static_cast<USize>(si.dwPageSize ? si.dwPageSize : 4096u);
    }

    [[nodiscard]] static USize QueryAllocationGranularity() noexcept {
        SYSTEM_INFO_MIN si{};
        GetSystemInfo(&si);
        return static_cast<USize>(si.dwAllocationGranularity ? si.dwAllocationGranularity : 64 * 1024u);
    }

    // Out-of-line implementations for Platform::Memory::VirtualMemory
    Expect<void*> Platform::Memory::VirtualMemory::Reserve(USize size) noexcept {
        const unsigned long prot = PAGE_NOACCESS; // 仅保留地址空间
        void* p = VirtualAlloc(nullptr, static_cast<UInt64>(size), MEM_RESERVE, prot);
        if (!p) return Expect<void*>::Err(MemErrWithOs(Memory::MemoryError::ReserveFailed, GetLastError()));
        return Expect<void*>::Ok(p);
    }

    Status Platform::Memory::VirtualMemory::Commit(void* base, USize size, Memory::PageProtection protection) noexcept {
        if (!base || size == 0) return MemErr(Memory::MemoryError::InvalidArgument);
        const unsigned long prot = ToWinProtect(protection);
        void* p = VirtualAlloc(base, static_cast<UInt64>(size), MEM_COMMIT, prot);
        return p ? Ok(StatusDomain::System()) : MemErrWithOs(Memory::MemoryError::CommitFailed, GetLastError());
    }

    Status Platform::Memory::VirtualMemory::Protect(void* base, USize size, Memory::PageProtection newProtection) noexcept {
        if (!base || size == 0) return MemErr(Memory::MemoryError::InvalidArgument);
        unsigned long oldProt = 0;
        const unsigned long prot = ToWinProtect(newProtection);
        const int ok = VirtualProtect(base, static_cast<UInt64>(size), prot, &oldProt);
        return ok ? Ok(StatusDomain::System()) : MemErrWithOs(Memory::MemoryError::ProtectFailed, GetLastError());
    }

    Status Platform::Memory::VirtualMemory::Decommit(void* base, USize size) noexcept {
        if (!base || size == 0) return MemErr(Memory::MemoryError::InvalidArgument);
        const int ok = VirtualFree(base, static_cast<unsigned long>(size), MEM_DECOMMIT);
        return ok ? Ok(StatusDomain::System()) : MemErrWithOs(Memory::MemoryError::DecommitFailed, GetLastError());
    }

    Status Platform::Memory::VirtualMemory::Release(void* base) noexcept {
        if (!base) return MemErr(Memory::MemoryError::InvalidArgument);
        const int ok = VirtualFree(base, 0u, MEM_RELEASE);
        return ok ? Ok(StatusDomain::System()) : MemErrWithOs(Memory::MemoryError::ReleaseFailed, GetLastError());
    }

    USize Platform::Memory::VirtualMemory::PageSize() noexcept { return QueryPageSize(); }
    USize Platform::Memory::VirtualMemory::AllocationGranularity() noexcept { return QueryAllocationGranularity(); }

    Expect<USize> Platform::Memory::VirtualMemory::LargePageSize() noexcept {
        const UInt64 lp = GetLargePageMinimum();
        if (lp == 0ull) {
            return Expect<USize>::Err(MemErr(Memory::MemoryError::LargePagesNotSupported));
        }
        return Expect<USize>::Ok(static_cast<USize>(lp));
    }

    Expect<void*> Platform::Memory::VirtualMemory::ReserveEx(USize size, UInt32 numaNodeId, bool useLargePages) noexcept {
        void* process = GetCurrentProcess();
        if (!process) return Expect<void*>::Err(MemErr(Memory::MemoryError::QueryFailed));
        unsigned long allocFlags = MEM_RESERVE;
        unsigned long prot = PAGE_NOACCESS;
        UInt64 requestSize = static_cast<UInt64>(size);
        if (useLargePages) {
            const UInt64 lp = GetLargePageMinimum();
            if (lp == 0) {
                return Expect<void*>::Err(MemErr(Memory::MemoryError::LargePagesNotSupported));
            }
            allocFlags |= MEM_COMMIT | MEM_LARGE_PAGES;
            prot = PAGE_READWRITE; // 大页必须提交且可访问
            const UInt64 mask = lp - 1ull;
            requestSize = (requestSize + mask) & ~mask;
        }
        void* p = VirtualAllocExNuma(process, nullptr, requestSize, allocFlags, prot, static_cast<unsigned long>(numaNodeId));
        if (!p) return Expect<void*>::Err(MemErrWithOs(Memory::MemoryError::ReserveFailed, GetLastError()));
        return Expect<void*>::Ok(p);
    }

    // --- Heap primitives ---
    extern "C" __declspec(dllimport) void* HeapCreate(unsigned long flOptions, UInt64 dwInitialSize, UInt64 dwMaximumSize);
    extern "C" __declspec(dllimport) int   HeapDestroy(void* hHeap);

    Expect<Memory::HeapHandle> Platform::Memory::Heap::Create() noexcept {
        void* h = HeapCreate(0u, 0ull, 0ull);
        if (!h) return Expect<Memory::HeapHandle>::Err(MemErrWithOs(Memory::MemoryError::HeapCreateFailed, GetLastError()));
        return Expect<Memory::HeapHandle>::Ok(Memory::HeapHandle{h});
    }

    Status Platform::Memory::Heap::Destroy(Memory::HeapHandle h) noexcept {
        if (!h.Get()) return MemErr(Memory::MemoryError::InvalidArgument);
        const int ok = HeapDestroy(h.Get());
        return ok ? Ok(StatusDomain::System()) : MemErrWithOs(Memory::MemoryError::HeapDestroyFailed, GetLastError());
    }

    Memory::HeapHandle Platform::Memory::Heap::GetProcessDefault() noexcept {
        return Memory::HeapHandle{GetProcessHeap()};
    }

    Expect<void*> Platform::Memory::Heap::AllocRaw(Memory::HeapHandle h, USize size) noexcept {
        if (!h.Get() || size == 0) return Expect<void*>::Err(MemErr(Memory::MemoryError::InvalidArgument));
        void* p = HeapAlloc(h.Get(), 0u, static_cast<UInt64>(size));
        if (!p) return Expect<void*>::Err(MemErrWithOs(Memory::MemoryError::HeapAllocFailed, GetLastError()));
        return Expect<void*>::Ok(p);
    }

    Status Platform::Memory::Heap::FreeRaw(Memory::HeapHandle h, void* p) noexcept {
        if (!h.Get() || !p) return MemErr(Memory::MemoryError::InvalidArgument);
        const int ok = HeapFree(h.Get(), 0u, p);
        return ok ? Ok(StatusDomain::System()) : MemErrWithOs(Memory::MemoryError::HeapFreeFailed, GetLastError());
    }

    Expect<void*> Platform::Memory::Heap::Alloc(Memory::HeapHandle h, USize size, USize alignment) noexcept {
        if (!h.Get() || size == 0) return Expect<void*>::Err(MemErr(Memory::MemoryError::InvalidArgument));
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
            return Expect<void*>::Err(MemErr(Memory::MemoryError::AlignmentNotPowerOfTwo));
        }
        if (alignment > Platform::Memory::Heap::MaximumAlignment()) {
            return Expect<void*>::Err(MemErr(Memory::MemoryError::AlignmentTooLarge));
        }
        const USize extra = alignment - 1 + sizeof(void*);
        const UInt64 req = static_cast<UInt64>(size) + static_cast<UInt64>(extra);
        if (req < static_cast<UInt64>(size)) {
            return Expect<void*>::Err(MemErr(Memory::MemoryError::OutOfMemory));
        }
        void* raw = HeapAlloc(h.Get(), 0u, req);
        if (!raw) return Expect<void*>::Err(MemErrWithOs(Memory::MemoryError::HeapAllocFailed, GetLastError()));
        uintptr_t base = reinterpret_cast<uintptr_t>(raw) + sizeof(void*);
        uintptr_t aligned = (base + (alignment - 1)) & ~(alignment - 1);
        void** marker = reinterpret_cast<void**>(aligned);
        marker[-1] = raw;
        return Expect<void*>::Ok(reinterpret_cast<void*>(aligned));
    }

    Status Platform::Memory::Heap::Free(Memory::HeapHandle h, void* p) noexcept {
        if (!h.Get() || !p) return MemErr(Memory::MemoryError::InvalidArgument);
        void** marker = reinterpret_cast<void**>(p);
        void* raw = marker[-1];
        const int ok = HeapFree(h.Get(), 0u, raw);
        return ok ? Ok(StatusDomain::System()) : MemErrWithOs(Memory::MemoryError::HeapFreeFailed, GetLastError());
    }

    USize Platform::Memory::Heap::MaximumAlignment() noexcept {
        // 建议上限：使用系统分配粒度以避免过度额外开销
        return QueryAllocationGranularity();
    }

    // Removed: FreeAligned (use handle-based Free)

    // Removed: Realloc (strategy-specific; not part of handle-based core API)
