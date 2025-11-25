module Memory;
import Language;
import Platform;
import :Definitions;
import :IMemoryResource;
import :SmallObjectAllocator;
import :PoolAllocator;

namespace Memory {
    SmallObjectAllocatorResource::SmallObjectAllocatorResource(USize perPoolCapacity, IMemoryResource* fallback) noexcept
        : m_poolCount(SizeClassTable::kCount), m_alignment(Alignment::Default), m_fallback(fallback) {
        auto h = Platform::Memory::Heap::GetProcessDefault();
        auto rArray = Platform::Memory::Heap::AllocRaw(h, sizeof(PoolAllocatorResource*) * m_poolCount);
        if (!rArray.IsOk()) { m_pools = nullptr; m_poolCount = 0; return; }
        m_pools = static_cast<PoolAllocatorResource**>(rArray.Value());
        for (USize i = 0; i < m_poolCount; ++i) {
            auto rObj = Platform::Memory::Heap::AllocRaw(h, sizeof(PoolAllocatorResource));
            if (!rObj.IsOk()) { m_pools[i] = nullptr; continue; }
            void* mem = rObj.Value();
            USize bs = SizeClassTable::ClassSize(i);
            new (mem) PoolAllocatorResource(bs, perPoolCapacity);
            m_pools[i] = static_cast<PoolAllocatorResource*>(mem);
        }
    }

    SmallObjectAllocatorResource::~SmallObjectAllocatorResource() noexcept {
        auto h = Platform::Memory::Heap::GetProcessDefault();
        if (m_pools) {
            for (USize i = 0; i < m_poolCount; ++i) {
                if (m_pools[i]) {
                    m_pools[i]->~PoolAllocatorResource();
                    (void)Platform::Memory::Heap::FreeRaw(h, m_pools[i]);
                }
            }
            (void)Platform::Memory::Heap::FreeRaw(h, m_pools);
        }
        m_pools = nullptr; m_poolCount = 0; m_fallback = nullptr;
    }

    Expect<MemoryBlock> SmallObjectAllocatorResource::Allocate(USize size, USize align) noexcept {
        if (size == 0) {
            return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        if (!Alignment::IsPowerOfTwo(align)) {
            return Expect<MemoryBlock>::Err(Platform::Memory::MemErr(Platform::Memory::MemoryError::AlignmentNotPowerOfTwo));
        }

        if (size <= SizeClassTable::kMaxSmall) {
            USize idx = SizeClassTable::ClassIndex(size);
            if (idx >= m_poolCount || !m_pools || !m_pools[idx]) {
                return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::Failed));
            }
            return m_pools[idx]->Allocate(size, align);
        }

        if (m_fallback) {
            return m_fallback->Allocate(size, align);
        }
        return Expect<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
    }

    void SmallObjectAllocatorResource::Deallocate(MemoryBlock block, USize align) noexcept {
        if (block.Empty()) return;
        if (block.size <= SizeClassTable::kMaxSmall) {
            USize idx = SizeClassTable::ClassIndex(block.size);
            if (idx < m_poolCount && m_pools && m_pools[idx]) {
                m_pools[idx]->Deallocate(block, align);
                return;
            }
        }
        if (m_fallback) {
            m_fallback->Deallocate(block, align);
        }
    }

    Expect<MemoryBlock> SmallObjectAllocatorResource::Reallocate(MemoryBlock block, USize newSize, USize align) noexcept {
        if (newSize == 0) {
            if (!block.Empty()) { Deallocate(block, align); }
            return Expect<MemoryBlock>::Ok(MemoryBlock{});
        }

        bool oldSmall = !block.Empty() && block.size <= SizeClassTable::kMaxSmall;
        bool newSmall = newSize <= SizeClassTable::kMaxSmall;

        if (oldSmall && newSmall) {
            USize oldIdx = SizeClassTable::ClassIndex(block.size);
            USize newIdx = SizeClassTable::ClassIndex(newSize);
            if (oldIdx == newIdx && m_pools && m_pools[oldIdx]) {
                return m_pools[oldIdx]->Reallocate(block, newSize, align);
            }
        }

        auto nb = Allocate(newSize, align);
        if (!nb.IsOk()) { return Expect<MemoryBlock>::Err(nb.Error()); }
        void* dst = nb.Value().ptr;
        const void* src = block.ptr;
        USize n = (block.size < newSize) ? block.size : newSize;
        MemMove(dst, src, n);
        Deallocate(block, align);
        return Expect<MemoryBlock>::Ok(MemoryBlock{ nb.Value().ptr, newSize });
    }

    bool SmallObjectAllocatorResource::IsEqual(const IMemoryResource& other) const noexcept {
        return reinterpret_cast<const void*>(this) == reinterpret_cast<const void*>(&other);
    }
}