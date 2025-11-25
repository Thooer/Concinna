export module Memory:SystemAllocator;

import Language;
import Platform;
import :Definitions;
import :IMemoryResource;

export namespace Memory {
    struct SystemMemoryResource : IMemoryResource {
        Platform::Memory::HeapHandle heap{};

        SystemMemoryResource() noexcept : heap(Platform::Memory::Heap::GetProcessDefault()) {}

        [[nodiscard]] StatusResult<MemoryBlock> Allocate(USize size, USize align) noexcept override {
            if (size == 0) {
                return StatusResult<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
            }
            if (!Alignment::IsPowerOfTwo(align)) {
                return StatusResult<MemoryBlock>::Err(Platform::Memory::MemErr(Platform::Memory::MemoryError::AlignmentNotPowerOfTwo));
            }
            if (align > Platform::Memory::Heap::MaximumAlignment()) {
                return StatusResult<MemoryBlock>::Err(Platform::Memory::MemErr(Platform::Memory::MemoryError::AlignmentTooLarge));
            }
            auto r = Platform::Memory::Heap::Alloc(heap, size, align);
            if (!r.IsOk()) {
                return StatusResult<MemoryBlock>::Err(r.Error());
            }
            return StatusResult<MemoryBlock>::Ok(MemoryBlock{ r.Value(), size });
        }

        void Deallocate(MemoryBlock block, USize) noexcept override {
            if (block.Empty()) return;
            (void)Platform::Memory::Heap::Free(heap, block.ptr);
        }

        [[nodiscard]] StatusResult<MemoryBlock> Reallocate(MemoryBlock block, USize newSize, USize align) noexcept override {
            if (newSize == 0) {
                if (!block.Empty()) { (void)Platform::Memory::Heap::Free(heap, block.ptr); }
                return StatusResult<MemoryBlock>::Ok(MemoryBlock{});
            }
            auto nr = Platform::Memory::Heap::Alloc(heap, newSize, align);
            if (!nr.IsOk()) {
                return StatusResult<MemoryBlock>::Err(nr.Error());
            }
            auto* dst = static_cast<void*>(nr.Value());
            auto* src = static_cast<const void*>(block.ptr);
            USize n = (block.size < newSize) ? block.size : newSize;
            MemMove(dst, src, n);
            if (!block.Empty()) { (void)Platform::Memory::Heap::Free(heap, block.ptr); }
            return StatusResult<MemoryBlock>::Ok(MemoryBlock{ nr.Value(), newSize });
        }

        [[nodiscard]] bool IsEqual(const IMemoryResource& other) const noexcept override {
            return reinterpret_cast<const void*>(this) == reinterpret_cast<const void*>(&other);
        }
    };
}