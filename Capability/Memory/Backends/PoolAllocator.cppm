module Cap.Memory;
import Language;
import Prm.Ownership;
import :Definitions;
import :IMemoryResource;
import :PoolAllocator;
import :ThreadCache;

namespace Cap {
    // 线程亲和PoolAllocator实现 - 优化热路径分配性能
    PoolAllocatorResource::PoolAllocatorResource(USize blockSize, USize capacity) noexcept 
        : m_capacity(capacity), m_committed(0), m_blockSize(blockSize), m_alignment(Alignment::Default), m_localHead(nullptr) {
        m_base = ::operator new(capacity);
        m_owner = ThreadMemory::Get();
        m_getContext = &ThreadMemory::Get;
        
        // 初始化统计信息
        m_totalAllocated.Store(0, MemoryOrder::Relaxed);
        m_totalFreed.Store(0, MemoryOrder::Relaxed);
        m_currentUsage.Store(0, MemoryOrder::Relaxed);
        m_peakUsage.Store(0, MemoryOrder::Relaxed);
        m_allocationCount.Store(0, MemoryOrder::Relaxed);
        m_freeCount.Store(0, MemoryOrder::Relaxed);
    }



    PoolAllocatorResource::~PoolAllocatorResource() noexcept {
        if (m_base) { ::operator delete(m_base); m_base = nullptr; }
        m_capacity = 0; m_committed = 0; m_blockSize = 0; m_alignment = Alignment::Default;
        m_remote.Reset();
        m_localHead = nullptr;
        m_owner = nullptr;
    }

    // 热路径：本地线程分配 - 完全无锁
    Expect<MemoryBlock> PoolAllocatorResource::Allocate(USize size, USize align) noexcept {
        if (!m_base || size == 0) {
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        if (size > m_blockSize) {
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
        }
        if (!Alignment::IsPowerOfTwo(align)) {
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        if (align > m_alignment) {
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
        }

        // 热路径：尝试从本地空闲链表分配（非原子，无锁）
        void* p = m_localHead;
        if (p) {
            m_localHead = reinterpret_cast<FreeNode*>(p)->next;
            
            // 更新分配统计信息
            m_totalAllocated.FetchAdd(size, MemoryOrder::Relaxed);
            m_currentUsage.FetchAdd(size, MemoryOrder::Relaxed);
            m_allocationCount.FetchAdd(1, MemoryOrder::Relaxed);
            
            // 更新峰值使用量
            auto current = m_currentUsage.Load(MemoryOrder::Relaxed);
            auto peak = m_peakUsage.Load(MemoryOrder::Relaxed);
            if (current > peak) {
                m_peakUsage.Store(current, MemoryOrder::Relaxed);
            }
            
            return Expect<MemoryBlock>::Ok(MemoryBlock{ p, size });
        }

        // 冷路径：本地为空，从远程链表偷取一批
        RefillLocal();
        p = m_localHead;
        if (p) {
            m_localHead = reinterpret_cast<FreeNode*>(p)->next;
            
            // 更新分配统计信息
            m_totalAllocated.FetchAdd(size, MemoryOrder::Relaxed);
            m_currentUsage.FetchAdd(size, MemoryOrder::Relaxed);
            m_allocationCount.FetchAdd(1, MemoryOrder::Relaxed);
            
            // 更新峰值使用量
            auto current = m_currentUsage.Load(MemoryOrder::Relaxed);
            auto peak = m_peakUsage.Load(MemoryOrder::Relaxed);
            if (current > peak) {
                m_peakUsage.Store(current, MemoryOrder::Relaxed);
            }
            
            return Expect<MemoryBlock>::Ok(MemoryBlock{ p, size });
        }

        // 仍然没有内存，向OS申请新的页面
        USize page = 4096;
        constexpr USize kBatchPages = 512;
        USize commitChunk = page * kBatchPages;
        USize commitTo = Alignment::AlignUp(m_committed + commitChunk, page);
        if (commitTo > m_capacity) { commitTo = m_capacity; }
        if (commitTo > m_capacity) {
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
        }
        
        USize start = Alignment::AlignUp(m_committed, page);
        USize toCommit = (commitTo > start) ? (commitTo - start) : 0;
        if (toCommit > 0) {
            void* base = static_cast<Byte*>(m_base) + start;
            // assume reserved memory usable; skip OS commit in this backend stub
            (void)base; (void)toCommit;
            m_committed = commitTo;
            
            // 将新分配的内存块放入本地空闲链表
            USize count = toCommit / m_blockSize;
            Byte* cur = static_cast<Byte*>(base);
            for (USize i = 0; i < count; ++i) {
                auto* node = reinterpret_cast<FreeNode*>(cur);
                node->next = m_localHead;
                m_localHead = node;
                cur += m_blockSize;
            }
        }
        
        p = m_localHead;
        if (p) {
            m_localHead = reinterpret_cast<FreeNode*>(p)->next;
            
            // 更新分配统计信息
            m_totalAllocated.FetchAdd(size, MemoryOrder::Relaxed);
            m_currentUsage.FetchAdd(size, MemoryOrder::Relaxed);
            m_allocationCount.FetchAdd(1, MemoryOrder::Relaxed);
            
            // 更新峰值使用量
            auto current = m_currentUsage.Load(MemoryOrder::Relaxed);
            auto peak = m_peakUsage.Load(MemoryOrder::Relaxed);
            if (current > peak) {
                m_peakUsage.Store(current, MemoryOrder::Relaxed);
            }
            
            return Expect<MemoryBlock>::Ok(MemoryBlock{ p, size });
        }
        
        return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
    }

    void PoolAllocatorResource::Deallocate(MemoryBlock block, USize) noexcept {
        if (block.Empty()) return;
        
        ThreadLocalContext* cur = m_getContext ? m_getContext() : ThreadMemory::Get();
        auto* node = reinterpret_cast<FreeNode*>(block.ptr);
        
        if (cur == m_owner) {
            // 所有者线程：放入本地空闲链表（非原子，无锁）
            node->next = m_localHead;
            m_localHead = node;
        } else {
            // 其他线程：放入远程空闲链表（原子操作）
            m_remote.Push(node);
        }
        
        // 更新释放统计信息
        m_totalFreed.FetchAdd(block.size, MemoryOrder::Relaxed);
        m_currentUsage.FetchSub(block.size, MemoryOrder::Relaxed);
        m_freeCount.FetchAdd(1, MemoryOrder::Relaxed);
        
        (void)block; (void)m_alignment;
    }

    // 从远程链表批量偷取到本地 - 仅在本地为空时调用
    void PoolAllocatorResource::RefillLocal() noexcept {
        auto* list = m_remote.StealAll();
        if (list) {
            // 反转链表顺序（保持LIFO语义）
            FreeNode* prev = nullptr;
            FreeNode* curr = list;
            while (curr) {
                FreeNode* next = curr->next;
                curr->next = prev;
                prev = curr;
                curr = next;
            }
            // prev现在是反转后链表的头部（原来链表的尾部）
            if (prev) { prev->next = m_localHead; }
            m_localHead = prev;
        }
    }

    // 将本地链表清空到远程（用于析构）
    void PoolAllocatorResource::FlushLocalToRemote() noexcept {
        if (m_localHead) {
            m_remote.PushList(m_localHead);
            m_localHead = nullptr;
        }
    }

    Expect<MemoryBlock> PoolAllocatorResource::Reallocate(MemoryBlock block, USize newSize, USize align) noexcept {
        if (newSize == 0) { return Expect<MemoryBlock>::Ok(MemoryBlock{}); }
        if (newSize <= m_blockSize) { return Expect<MemoryBlock>::Ok(MemoryBlock{ block.ptr, newSize }); }
        auto nb = Allocate(newSize, align);
        if (!nb.IsOk()) { return Expect<MemoryBlock>::Err(nb.Error()); }
        void* dst = nb.Value().ptr;
        const void* src = block.ptr;
        USize n = (m_blockSize < newSize) ? m_blockSize : newSize;
        MemMove(dst, src, n);
        Deallocate(block, m_alignment);
        return Expect<MemoryBlock>::Ok(MemoryBlock{ nb.Value().ptr, newSize });
    }

    bool PoolAllocatorResource::IsEqual(const IMemoryResource& other) const noexcept {
        return reinterpret_cast<const void*>(this) == reinterpret_cast<const void*>(&other);
    }
    bool PoolAllocatorResource::IsOwnedByCurrentThread() const noexcept {
        ThreadLocalContext* cur = m_getContext ? m_getContext() : ThreadMemory::Get();
        return cur == m_owner;
    }
    void PoolAllocatorResource::Reset() noexcept {
        // 将本地链表清空到远程，避免持有线程外可见状态；偏向安全重置
        FlushLocalToRemote();
    }
    
    // 统计信息获取
    auto PoolAllocatorResource::GetStats() const noexcept -> AllocatorStats {
        AllocatorStats stats{};
        stats.totalAllocated = m_totalAllocated.Load(MemoryOrder::Relaxed);
        stats.totalFreed = m_totalFreed.Load(MemoryOrder::Relaxed);
        stats.currentUsage = m_currentUsage.Load(MemoryOrder::Relaxed);
        stats.peakUsage = m_peakUsage.Load(MemoryOrder::Relaxed);
        stats.allocationCount = m_allocationCount.Load(MemoryOrder::Relaxed);
        stats.freeCount = m_freeCount.Load(MemoryOrder::Relaxed);
        stats.allocationSize = m_blockSize;
        stats.totalBlocks = m_capacity / m_blockSize;
        
        // 计算空闲块数量
        USize localBlocks = 0;
        FreeNode* local = m_localHead;
        while (local) {
            localBlocks++;
            local = local->next;
        }
        
        // 远程链表的空闲块数量需要遍历
        USize remoteBlocks = 0;
        FreeNode* remote = m_remote.Head();
        while (remote) {
            remoteBlocks++;
            remote = remote->next;
        }
        
        stats.freeBlocks = localBlocks + remoteBlocks;
        stats.alignedSize = m_blockSize;
        
        return stats;
    }
    
    auto PoolAllocatorResource::ResetStats() noexcept -> void {
        m_totalAllocated.Store(0, MemoryOrder::Relaxed);
        m_totalFreed.Store(0, MemoryOrder::Relaxed);
        m_currentUsage.Store(0, MemoryOrder::Relaxed);
        m_peakUsage.Store(0, MemoryOrder::Relaxed);
        m_allocationCount.Store(0, MemoryOrder::Relaxed);
        m_freeCount.Store(0, MemoryOrder::Relaxed);
    }
    
    auto PoolAllocatorResource::DumpStats() const noexcept -> void {
        auto stats = GetStats();
        
        DebugBreak();  // 调用调试断点，便于调试时查看统计信息
        
        // 打印统计信息（使用占位符，实际需要Logging系统）
        (void)stats;  // 避免未使用变量警告
        
        // 这里应该使用Logging系统输出详细信息
        // 为了简化，先将逻辑实现放在这里
    }
    
    namespace Sentinel {
        auto VerifyFrontSentinel(const void* block, USize offset) noexcept -> bool {
            if (!block) return false;
            auto* sentinel = reinterpret_cast<const UInt32*>(
                reinterpret_cast<const Byte*>(block) - static_cast<SSize>(offset) - static_cast<SSize>(sizeof(UInt32))
            );
            return *sentinel == FrontSentinel;
        }
        
        auto VerifyBackSentinel(const void* block, USize size) noexcept -> bool {
            if (!block) return false;
            auto* sentinel = reinterpret_cast<const UInt32*>(
                reinterpret_cast<const Byte*>(block) + static_cast<SSize>(size)
            );
            return *sentinel == BackSentinel;
        }
        
        auto MarkFreedSentinel(const void* block, USize offset, USize size) noexcept -> void {
            if (!block) return;
            auto* data = reinterpret_cast<UInt32*>(
                reinterpret_cast<Byte*>(const_cast<void*>(block)) + static_cast<SSize>(offset)
            );
            auto* end = reinterpret_cast<UInt32*>(
                reinterpret_cast<Byte*>(const_cast<void*>(block)) + static_cast<SSize>(size) - static_cast<SSize>(sizeof(UInt32))
            );
            
            #if defined(CONFIG_DEBUG) || defined(_DEBUG)
            // 在调试模式下，填充释放哨兵值
            while (data <= end) {
                *data = FreedSentinel;
                data = reinterpret_cast<UInt32*>(
                    reinterpret_cast<Byte*>(data) + static_cast<SSize>(sizeof(UInt32))
                );
            }
            #endif
        }
    }
}
