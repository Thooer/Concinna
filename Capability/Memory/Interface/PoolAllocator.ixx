export module Cap.Memory:PoolAllocator;

import Language;
import :Definitions;
import :IMemoryResource;
import :ThreadCache;
import :ConcurrentContainers;

export namespace Cap {
    // 哨兵值定义 - 用于内存越界检测
    namespace Sentinel {
        constexpr UInt32 FrontSentinel = 0xBADC0FFE;  // 前置哨兵
        constexpr UInt32 BackSentinel  = 0x0BADF00D;  // 后置哨兵
        constexpr UInt32 FreedSentinel = 0xDEADBEEF;  // 释放哨兵
        
        constexpr USize SentinelSize = sizeof(UInt32);
        
        // 检查哨兵值完整性
        auto VerifyFrontSentinel(const void* block, USize offset) noexcept -> bool;
        auto VerifyBackSentinel(const void* block, USize size) noexcept -> bool;
        auto MarkFreedSentinel(const void* block, USize offset, USize size) noexcept -> void;
    }
    
    // 分配器统计信息
    export struct AllocatorStats {
        USize totalAllocated{0};    // 已分配总字节数
        USize totalFreed{0};        // 已释放总字节数
        USize currentUsage{0};      // 当前使用字节数
        USize peakUsage{0};         // 峰值使用字节数
        USize allocationCount{0};   // 分配次数
        USize freeCount{0};         // 释放次数
        USize allocationSize{0};    // 分配单元大小
        USize totalBlocks{0};       // 总块数
        USize freeBlocks{0};        // 空闲块数
        USize alignedSize{0};       // 对齐后大小
        
        // 计算分配器利用率
        [[nodiscard]] constexpr auto GetUtilization() const noexcept -> Float32 {
            return totalBlocks > 0 ? static_cast<Float32>(totalBlocks - freeBlocks) / totalBlocks : 0.0f;
        }
        
        // 计算内存效率
        [[nodiscard]] constexpr auto GetMemoryEfficiency() const noexcept -> Float32 {
            return currentUsage > 0 ? static_cast<Float32>(totalAllocated) / currentUsage : 0.0f;
        }
    };
 
    // 线程亲和的内存分配器，优化单线程分配性能
    export struct PoolAllocatorResource : IMemoryResource {
        void* m_base{nullptr};
        USize m_capacity{0};
        USize m_committed{0};
        USize m_blockSize{0};
        USize m_alignment{Alignment::Default};
        
        // 无锁空闲链表节点定义
        struct FreeNode { FreeNode* next; };
        
        // 线程本地空闲链表 - 热路径完全无锁
        FreeNode* m_localHead{nullptr};  // 非原子，热路径专用
        
        using LockFreeFreeList = IntrusiveLockFreeStack<FreeNode>;
        LockFreeFreeList m_remote{};
        ThreadLocalContext* m_owner{nullptr};
        ThreadLocalContext* (*m_getContext)() noexcept = nullptr;
        
        // 统计信息成员 - 原子变量确保线程安全
        mutable Atomic<USize> m_totalAllocated{0};
        mutable Atomic<USize> m_totalFreed{0};
        mutable Atomic<USize> m_currentUsage{0};
        mutable Atomic<USize> m_peakUsage{0};
        mutable Atomic<USize> m_allocationCount{0};
        mutable Atomic<USize> m_freeCount{0};

        explicit PoolAllocatorResource(USize blockSize, USize capacity) noexcept;
        ~PoolAllocatorResource() noexcept;

        [[nodiscard]] StatusResult<MemoryBlock> Allocate(USize size, USize align) noexcept override;
        void Deallocate(MemoryBlock block, USize align) noexcept override;
        [[nodiscard]] StatusResult<MemoryBlock> Reallocate(MemoryBlock block, USize newSize, USize align) noexcept override;
        [[nodiscard]] bool IsEqual(const IMemoryResource& other) const noexcept override;
        [[nodiscard]] bool IsOwnedByCurrentThread() const noexcept override;
        void Reset() noexcept override;
        
        // 统计信息获取
        [[nodiscard]] auto GetStats() const noexcept -> AllocatorStats;
        auto ResetStats() noexcept -> void;
        auto DumpStats() const noexcept -> void;
        
        // 内部方法：批量从远程偷取到本地
        void RefillLocal() noexcept;
        
        // 内部方法：将本地链表完全清空到远程（用于 shutdown）
        void FlushLocalToRemote() noexcept;

        void SetOwner(ThreadLocalContext* owner) noexcept { m_owner = owner; }
        void SetContextProvider(ThreadLocalContext* (*getCtx)() noexcept) noexcept { m_getContext = getCtx; }
    };
}
