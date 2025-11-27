module Cap.Memory;
import Language;
import Prm.Ownership;
import :Definitions;
import :IMemoryResource;
import :NumaArena;

namespace Cap {
    NumaArenaResource::NumaArenaResource(USize capacity, UInt32 numaNode, bool useLargePages) noexcept
        : m_capacity(capacity), m_offset(0), m_committed(0), m_numaNode(numaNode), m_largePages(useLargePages) {
        m_base = ::operator new(capacity);
        if (m_base) {
            if (useLargePages) { m_committed = capacity; }
        }
    }

    NumaArenaResource::~NumaArenaResource() noexcept {
        if (m_base) { ::operator delete(m_base); m_base = nullptr; }
        m_capacity = 0; m_offset = 0; m_committed = 0; m_numaNode = 0; m_largePages = false;
    }

    Expect<MemoryBlock> NumaArenaResource::Allocate(USize size, USize align) noexcept {
        if (!m_base || size == 0) {
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        if (!Alignment::IsPowerOfTwo(align)) {
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        USize aligned = Alignment::AlignUp(m_offset, align);
        if (aligned + size > m_capacity) {
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
        }
        USize end = aligned + size;
        if (!m_largePages && end > m_committed) {
            USize page = 4096;
            USize commitTo = Alignment::AlignUp(end, page);
            USize start = Alignment::AlignUp(m_committed, page);
            USize toCommit = (commitTo > start) ? (commitTo - start) : 0;
            if (toCommit > 0) { m_committed = commitTo; }
        }
        auto* ptr = static_cast<Byte*>(m_base) + aligned;
        m_offset = end;
        return Expect<MemoryBlock>::Ok(MemoryBlock{ ptr, size });
    }

    void NumaArenaResource::Deallocate(MemoryBlock, USize) noexcept {}

    Expect<MemoryBlock> NumaArenaResource::Reallocate(MemoryBlock block, USize newSize, USize align) noexcept {
        if (newSize == 0) { return Expect<MemoryBlock>::Ok(MemoryBlock{}); }
        UIntPtr endOfBlock = reinterpret_cast<UIntPtr>(block.ptr) + block.size;
        UIntPtr currentTop = reinterpret_cast<UIntPtr>(m_base) + m_offset;
        if (endOfBlock == currentTop) {
            UIntPtr startOfBlock = reinterpret_cast<UIntPtr>(block.ptr);
            USize newEndOffset = static_cast<USize>(startOfBlock - reinterpret_cast<UIntPtr>(m_base)) + newSize;
            if (newEndOffset > m_capacity) {
                return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
            }
            if (!m_largePages && newEndOffset > m_committed) {
                USize page = 4096;
                USize commitTo = Alignment::AlignUp(newEndOffset, page);
                USize start = Alignment::AlignUp(m_committed, page);
                USize toCommit = (commitTo > start) ? (commitTo - start) : 0;
                if (toCommit > 0) { m_committed = commitTo; }
            }
            m_offset = newEndOffset;
            return Expect<MemoryBlock>::Ok(MemoryBlock{ block.ptr, newSize });
        }
        return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }

    bool NumaArenaResource::IsEqual(const IMemoryResource& other) const noexcept {
        return reinterpret_cast<const void*>(this) == reinterpret_cast<const void*>(&other);
    }

    void NumaArenaResource::Reset() noexcept { m_offset = 0; }
    void NumaArenaResource::ResetTo(USize off) noexcept { if (off <= m_capacity) m_offset = off; }
}
