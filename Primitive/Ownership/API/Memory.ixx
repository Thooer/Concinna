// Platform.Memory — 平台层内存接口（无导出宏，纯接口）
module;
export module Platform:Memory;

import Prm;

export namespace Platform::Memory {
    

    struct HeapHandleTag;
    export using HeapHandle = StrongAlias<void*, HeapHandleTag>;

    // 对齐辅助
    [[nodiscard]] constexpr USize AlignUp(USize value, USize align) noexcept {
        return (align > 0) ? ((value + (align - 1)) & ~(align - 1)) : value;
    }

    // 详细错误码（仅对 Memory 域使用；低16位可编码 OS 错误）
    export enum class MemoryError : Int32 {
        InvalidArgument       = 1,
        ReserveFailed         = 2,
        CommitFailed          = 3,
        ProtectFailed         = 4,
        DecommitFailed        = 5,
        ReleaseFailed         = 6,
        HeapCreateFailed      = 7,
        HeapDestroyFailed     = 8,
        HeapAllocFailed       = 9,
        HeapFreeFailed        = 10,
        AlignmentNotPowerOfTwo= 11,
        AlignmentTooLarge     = 12,
        SizeNotAligned        = 13,
        LargePagesNotSupported= 14,
        LargePagesNotEnabled  = 15,
        NumaUnavailable       = 16,
        QueryFailed           = 17,
        AccessDenied          = 18,
        OutOfMemory           = 19,
    };

    // 将 MemoryError 封装为 Core::Status（域：Memory）。
    [[nodiscard]] inline constexpr Status MemErr(MemoryError e, const char* msg = nullptr) noexcept {
        return Err(StatusDomain::System(), static_cast<StatusCodeValue>(e), msg);
    }
    // 可选：将 OS 错误码压入 StatusCode 低16位（高位为 MemoryError）。
    [[nodiscard]] inline constexpr Status MemErrWithOs(MemoryError e, UInt32 osErr, const char* msg = nullptr) noexcept {
        const StatusCodeValue packed = (static_cast<StatusCodeValue>(e) << 16) | static_cast<StatusCodeValue>(osErr & 0xFFFFu);
        return Err(StatusDomain::System(), packed, msg);
    }

    // 页面保护（平台通用抽象）
    export enum class PageProtection : UInt32 {
        NoAccess,
        ReadOnly,
        ReadWrite,
        ExecuteRead,
        ExecuteReadWrite,
    };

    // 虚拟内存原语（平台控制层）：Reserve/Commit/Decommit/Release
    export struct VirtualMemory {
        [[nodiscard]] static Expect<void*> Reserve(USize size) noexcept;
        [[nodiscard]] static Status Commit(void* base, USize size, PageProtection protection = PageProtection::ReadWrite) noexcept;
        [[nodiscard]] static Status Protect(void* base, USize size, PageProtection newProtection) noexcept;
        [[nodiscard]] static Status Decommit(void* base, USize size) noexcept;
        [[nodiscard]] static Status Release(void* base) noexcept;

        [[nodiscard]] static USize PageSize() noexcept;
        [[nodiscard]] static USize AllocationGranularity() noexcept;

        // 查询：系统大页尺寸（若不支持则返回 Err(LargePagesNotSupported)）
        [[nodiscard]] static Expect<USize> LargePageSize() noexcept;

        // 高级预留：支持 NUMA 和大页
        [[nodiscard]] static Expect<void*> ReserveEx(USize size, UInt32 numaNodeId, bool useLargePages) noexcept;
    };

    // 进程堆原语（堆隔离 + 对齐分配）
    export struct Heap {
        [[nodiscard]] static Expect<HeapHandle> Create() noexcept;
        [[nodiscard]] static Status Destroy(HeapHandle h) noexcept;
        [[nodiscard]] static HeapHandle GetProcessDefault() noexcept;

        [[nodiscard]] static Expect<void*> AllocRaw(HeapHandle h, USize size) noexcept;
        [[nodiscard]] static Status FreeRaw(HeapHandle h, void* p) noexcept;

        [[nodiscard]] static Expect<void*> Alloc(HeapHandle h, USize size, USize alignment) noexcept;
        [[nodiscard]] static Status Free(HeapHandle h, void* p) noexcept;

        // 查询：堆支持的建议最大对齐（平台相关）
        [[nodiscard]] static USize MaximumAlignment() noexcept;
    };
}
