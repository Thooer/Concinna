module Memory;
import Language;
import Platform;
import :Definitions;
import :IMemoryResource;
import :StackAllocator;

namespace Memory {
    StackAllocatorResource::StackAllocatorResource(USize capacity) noexcept : m_capacity(capacity), m_offset(0), m_committed(0) {
        auto r = Platform::Memory::VirtualMemory::Reserve(capacity);
        if (r.IsOk()) {
            m_base = r.Value();
            
        }
    }
    StackAllocatorResource::~StackAllocatorResource() noexcept {
        if (m_base) { (void)Platform::Memory::VirtualMemory::Release(m_base); m_base = nullptr; }
        m_capacity = 0; m_offset = 0;
    }
    Expect<MemoryBlock> StackAllocatorResource::Allocate(USize size, USize align) noexcept {
        if (!m_base || size == 0) {
            #if defined(CONFIG_DEBUG) || defined(_DEBUG)
            if (!m_base) DebugBreak();  // 分配器未初始化
            if (size == 0) DebugBreak();  // 分配0字节
            #endif
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        if (!Alignment::IsPowerOfTwo(align)) {
            return Expect<MemoryBlock>::Err(Platform::Memory::MemErr(Platform::Memory::MemoryError::AlignmentNotPowerOfTwo));
        }
        
        #if defined(CONFIG_DEBUG) || defined(_DEBUG)
        if (align == 0) DebugBreak();  // 对齐不能为0
        if (align > m_capacity) DebugBreak();  // 对齐值过大
        if (m_offset > m_committed) DebugBreak();  // 状态不一致
        #endif
        
        USize aligned = Alignment::AlignUp(m_offset, align);
        
        // 边界检查
        #if defined(CONFIG_DEBUG) || defined(_DEBUG)
        if (aligned < m_offset) DebugBreak();  // 对齐计算错误
        if (aligned + size < aligned) DebugBreak();  // 溢出
        #endif
        
        if (aligned + size > m_capacity) {
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
        }
        
        USize end = aligned + size;
        
        #if defined(CONFIG_DEBUG) || defined(_DEBUG)
        if (end <= aligned) DebugBreak();  // 溢出检查
        #endif
        
        if (end > m_committed) {
            USize page = Platform::Memory::VirtualMemory::PageSize();
            USize commitTo = Alignment::AlignUp(end, page);
            USize start = Alignment::AlignUp(m_committed, page);
            USize toCommit = (commitTo > start) ? (commitTo - start) : 0;
            if (toCommit > 0) {
                void* base = static_cast<Byte*>(m_base) + start;
                Status s = Platform::Memory::VirtualMemory::Commit(base, toCommit);
                if (!s.Ok()) {
                    return Expect<MemoryBlock>::Err(s);
                }
                m_committed = commitTo;
                
                #if defined(CONFIG_DEBUG) || defined(_DEBUG)
                if (m_committed != commitTo) DebugBreak();  // 状态不一致
                #endif
            }
        }
        
        auto* ptr = static_cast<Byte*>(m_base) + aligned;
        USize newOffset = aligned + size;
        
        #if defined(CONFIG_DEBUG) || defined(_DEBUG)
        if (newOffset <= m_offset) DebugBreak();  // 偏移量错误
        if (newOffset > m_committed) DebugBreak();  // 状态不一致
        if (ptr < m_base) DebugBreak();  // 指针越界
        #endif
        
        m_offset = newOffset;
        return Expect<MemoryBlock>::Ok(MemoryBlock{ptr, size});
    }
    void StackAllocatorResource::Deallocate(MemoryBlock block, USize) noexcept {
        if (!m_base || block.Empty()) {
            #if defined(CONFIG_DEBUG) || defined(_DEBUG)
            if (!m_base) DebugBreak();  // 分配器未初始化
            if (block.Empty()) DebugBreak();  // 尝试释放空块
            #endif
            return;
        }
        
        // 边界验证：检查block是否在有效范围内
        #if defined(CONFIG_DEBUG) || defined(_DEBUG)
        auto* blockStart = static_cast<Byte*>(block.ptr);
        auto* blockEnd = blockStart + block.size;
        auto* allocStart = static_cast<Byte*>(m_base);
        auto* allocEnd = allocStart + m_capacity;
        
        if (blockStart < allocStart || blockEnd > allocEnd) {
            DebugBreak();  // 试图释放超出分配器范围的内存
        }
        if (block.size == 0) {
            DebugBreak();  // 释放大小为0的块
        }
        #endif
        
        auto* top = static_cast<Byte*>(m_base) + m_offset;
        auto* end = static_cast<Byte*>(block.ptr) + block.size;
        
        if (end == top) {
            m_offset = static_cast<USize>(reinterpret_cast<UIntPtr>(block.ptr) - reinterpret_cast<UIntPtr>(m_base));
        } else {
            #if defined(CONFIG_DEBUG) || defined(_DEBUG)
            DebugBreak();  // 栈分配器必须按LIFO顺序释放！当前尝试释放的块不是栈顶元素
            #endif
            // 静默忽略非栈顶释放（保持原有行为）
        }
    }
    Expect<MemoryBlock> StackAllocatorResource::Reallocate(MemoryBlock block, USize newSize, USize align) noexcept {
        if (newSize == 0) { return Expect<MemoryBlock>::Ok(MemoryBlock{}); }
        UIntPtr endOfBlock = reinterpret_cast<UIntPtr>(block.ptr) + block.size;
        UIntPtr currentTop = reinterpret_cast<UIntPtr>(m_base) + m_offset;
        if (endOfBlock == currentTop) {
            UIntPtr startOfBlock = reinterpret_cast<UIntPtr>(block.ptr);
            USize newEndOffset = static_cast<USize>(startOfBlock - reinterpret_cast<UIntPtr>(m_base)) + newSize;
            if (newEndOffset > m_capacity) {
                return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
            }
            if (newEndOffset > m_committed) {
                USize page = Platform::Memory::VirtualMemory::PageSize();
                USize commitTo = Alignment::AlignUp(newEndOffset, page);
                USize start = Alignment::AlignUp(m_committed, page);
                USize toCommit = (commitTo > start) ? (commitTo - start) : 0;
                if (toCommit > 0) {
                    void* base = static_cast<Byte*>(m_base) + start;
                    Status s = Platform::Memory::VirtualMemory::Commit(base, toCommit);
                    if (!s.Ok()) {
                        return Expect<MemoryBlock>::Err(s);
                    }
                    m_committed = commitTo;
                }
            }
            m_offset = newEndOffset;
            return Expect<MemoryBlock>::Ok(MemoryBlock{ block.ptr, newSize });
        }
        return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    bool StackAllocatorResource::IsEqual(const IMemoryResource& other) const noexcept {
        return reinterpret_cast<const void*>(this) == reinterpret_cast<const void*>(&other);
    }
    void StackAllocatorResource::ResetTo(USize off) noexcept {
        if (off <= m_capacity) m_offset = off;
    }
}