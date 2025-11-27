module Cap.Memory;
import Language;
import Prm.Ownership;
import :Definitions;
import :IMemoryResource;
import :FrameAllocator;

namespace Cap {
    FrameAllocatorResource::FrameAllocatorResource(USize capacity) noexcept : m_capacity(capacity), m_offset(0), m_committed(0) {
        m_base = ::operator new(capacity);
    }
    FrameAllocatorResource::~FrameAllocatorResource() noexcept {
        if (m_base) { ::operator delete(m_base); m_base = nullptr; }
        m_capacity = 0; m_offset = 0;
    }
    Expect<MemoryBlock> FrameAllocatorResource::Allocate(USize size, USize align) noexcept {
        if (!m_base || size == 0) {
            #if defined(CONFIG_DEBUG) || defined(_DEBUG)
            if (!m_base) DebugBreak();  // 分配器未初始化
            if (size == 0) DebugBreak();  // 分配0字节
            #endif
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        if (!Alignment::IsPowerOfTwo(align)) {
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        
        #if defined(CONFIG_DEBUG) || defined(_DEBUG)
        if (align == 0) DebugBreak();  // 对齐不能为0
        if (align > m_capacity) DebugBreak();  // 对齐值过大
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
        if (end > m_committed && end > m_capacity) DebugBreak();  // 超出总容量
        #endif
        
        if (end > m_committed) {
            USize page = 4096;
            USize commitTo = Alignment::AlignUp(end, page);
            USize start = Alignment::AlignUp(m_committed, page);
            USize toCommit = (commitTo > start) ? (commitTo - start) : 0;
            if (toCommit > 0) { m_committed = commitTo; }
        }
        
        auto* ptr = static_cast<Byte*>(m_base) + aligned;
        USize newOffset = aligned + size;
        
        #if defined(CONFIG_DEBUG) || defined(_DEBUG)
        if (newOffset <= m_offset) DebugBreak();  // 偏移量错误
        if (newOffset > m_committed) DebugBreak();  // 状态不一致
        #endif
        
        m_offset = newOffset;
        return Expect<MemoryBlock>::Ok(MemoryBlock{ptr, size});
    }
    void FrameAllocatorResource::Deallocate(MemoryBlock block, USize) noexcept { 
        // FrameAllocator 不支持单个释放，仅支持Reset()
        // 在Debug模式下警告用户
        #if defined(CONFIG_DEBUG) || defined(_DEBUG)
        if (!block.Empty()) {
            DebugBreak();  // 框架分配器不支持单个释放，使用Reset()
        }
        #endif
    }
    Expect<MemoryBlock> FrameAllocatorResource::Reallocate(MemoryBlock block, USize newSize, USize align) noexcept {
        if (newSize == 0) { return Expect<MemoryBlock>::Ok(MemoryBlock{}); }
        
        // 边界检查和调试断言
        #if defined(CONFIG_DEBUG) || defined(_DEBUG)
        if (!m_base) {
            DebugBreak();  // 分配器未初始化
        }
        if (block.Empty()) {
            DebugBreak();  // 空块
        }
        if (newSize > m_capacity) {
            DebugBreak();  // 新大小超过容量
        }
        #endif
        
        UIntPtr endOfBlock = reinterpret_cast<UIntPtr>(block.ptr) + block.size;
        UIntPtr currentTop = reinterpret_cast<UIntPtr>(m_base) + m_offset;
        
        // 只能重分配栈顶的块
        if (endOfBlock == currentTop) {
            UIntPtr startOfBlock = reinterpret_cast<UIntPtr>(block.ptr);
            USize newEndOffset = static_cast<USize>(startOfBlock - reinterpret_cast<UIntPtr>(m_base)) + newSize;
            
            #if defined(CONFIG_DEBUG) || defined(_DEBUG)
            if (newEndOffset > m_capacity) {
                DebugBreak();  // 重分配后超出容量
                return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
            }
            #else
            if (newEndOffset > m_capacity) {
                return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
            }
            #endif
            
            if (newEndOffset > m_committed) {
                USize page = 4096;
                USize commitTo = Alignment::AlignUp(newEndOffset, page);
                USize start = Alignment::AlignUp(m_committed, page);
                USize toCommit = (commitTo > start) ? (commitTo - start) : 0;
                if (toCommit > 0) { m_committed = commitTo; }
            }
            
            #if defined(CONFIG_DEBUG) || defined(_DEBUG)
            // 验证新的偏移量在合理范围内
            if (newEndOffset > m_capacity || newEndOffset < m_offset) {
                DebugBreak();  // 计算错误
            }
            #endif
            
            m_offset = newEndOffset;
            return Expect<MemoryBlock>::Ok(MemoryBlock{ block.ptr, newSize });
        }
        
        #if defined(CONFIG_DEBUG) || defined(_DEBUG)
        DebugBreak();  // 尝试重分配非栈顶块
        #endif
        
        return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    bool FrameAllocatorResource::IsEqual(const IMemoryResource& other) const noexcept {
        return reinterpret_cast<const void*>(this) == reinterpret_cast<const void*>(&other);
    }
    void FrameAllocatorResource::Reset() noexcept { m_offset = 0; }
    void FrameAllocatorResource::ResetTo(USize off) noexcept { if (off <= m_capacity) m_offset = off; }
}
