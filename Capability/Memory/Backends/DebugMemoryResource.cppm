module Memory;
import Language;
import Platform;
import :Definitions;
import :IMemoryResource;
import :DebugMemoryResource;

namespace Memory {
    static constexpr UInt32 kCanary = 0xDEADBEEF;

    DebugMemoryResource::DebugMemoryResource(IMemoryResource* inner) noexcept : m_inner(inner), m_head(nullptr) {
        m_count.Store(0, MemoryOrder::Relaxed);
    }

    DebugMemoryResource::~DebugMemoryResource() noexcept {
        if (m_head) {
            DebugBreak();
        }
        m_inner = nullptr;
    }

    Expect<MemoryBlock> DebugMemoryResource::Allocate(USize size, USize align) noexcept {
        if (!m_inner || size == 0) {
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        if (!Alignment::IsPowerOfTwo(align)) {
            return Expect<MemoryBlock>::Err(Platform::Memory::MemErr(Platform::Memory::MemoryError::AlignmentNotPowerOfTwo));
        }

        USize extra = static_cast<USize>(sizeof(UInt32)) * 2 + align;
        auto r = m_inner->Allocate(size + extra, align);
        if (!r.IsOk()) { return Expect<MemoryBlock>::Err(r.Error()); }
        MemoryBlock baseBlock = r.Value();
        auto* raw = static_cast<Byte*>(baseBlock.ptr);
        UIntPtr p = reinterpret_cast<UIntPtr>(raw + static_cast<SSize>(sizeof(UInt32)));
        UIntPtr aligned = Alignment::AlignUp(p, align);
        auto* user = reinterpret_cast<void*>(aligned);

        auto* front = reinterpret_cast<UInt32*>(reinterpret_cast<Byte*>(user) - static_cast<SSize>(sizeof(UInt32)));
        auto* back = reinterpret_cast<UInt32*>(reinterpret_cast<Byte*>(user) + static_cast<SSize>(size));
        *front = kCanary;
        *back = kCanary;

        auto h = Platform::Memory::Heap::GetProcessDefault();
        auto rn = Platform::Memory::Heap::AllocRaw(h, sizeof(Entry));
        if (!rn.IsOk()) {
            m_inner->Deallocate(baseBlock, align);
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        void* nm = rn.Value();
        auto* e = new (nm) Entry{ user, baseBlock.ptr, size, baseBlock.size, align, nullptr };
        e->next = m_head;
        m_head = e;
        m_count.FetchAdd(1, MemoryOrder::Relaxed);

        return Expect<MemoryBlock>::Ok(MemoryBlock{ user, size });
    }

    void DebugMemoryResource::Deallocate(MemoryBlock block, USize align) noexcept {
        if (block.Empty()) return;
        Entry* prev = nullptr;
        Entry* cur = m_head;
        while (cur && cur->user != block.ptr) { prev = cur; cur = cur->next; }
        if (!cur) {
            DebugBreak();
            return;
        }
        auto* front = reinterpret_cast<const UInt32*>(reinterpret_cast<const Byte*>(block.ptr) - static_cast<SSize>(sizeof(UInt32)));
        auto* back = reinterpret_cast<const UInt32*>(reinterpret_cast<const Byte*>(block.ptr) + static_cast<SSize>(cur->requested));
        if (*front != kCanary || *back != kCanary) {
            DebugBreak();
        }
        if (prev) prev->next = cur->next; else m_head = cur->next;
        m_count.FetchSub(1, MemoryOrder::Relaxed);

        m_inner->Deallocate(MemoryBlock{ cur->base, cur->total }, cur->align);
        auto h = Platform::Memory::Heap::GetProcessDefault();
        cur->~Entry();
        (void)Platform::Memory::Heap::FreeRaw(h, cur);
    }

    Expect<MemoryBlock> DebugMemoryResource::Reallocate(MemoryBlock block, USize newSize, USize align) noexcept {
        if (newSize == 0) {
            if (!block.Empty()) { Deallocate(block, align); }
            return Expect<MemoryBlock>::Ok(MemoryBlock{});
        }
        auto nb = Allocate(newSize, align);
        if (!nb.IsOk()) { return Expect<MemoryBlock>::Err(nb.Error()); }
        void* dst = nb.Value().ptr;
        const void* src = block.ptr;
        USize n = ((block.size < newSize) ? block.size : newSize);
        MemMove(dst, src, n);
        Deallocate(block, align);
        return Expect<MemoryBlock>::Ok(MemoryBlock{ nb.Value().ptr, newSize });
    }

    bool DebugMemoryResource::IsEqual(const IMemoryResource& other) const noexcept {
        return reinterpret_cast<const void*>(this) == reinterpret_cast<const void*>(&other);
    }
}