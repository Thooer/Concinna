export module Cap.Memory:SystemAllocator;

import Lang;
import Lang.Flow;
import Prm.Ownership;
import <new>;
import :Definitions;
import :IMemoryResource;

export namespace Cap {
    struct SystemMemoryResource : IMemoryResource {
        SystemMemoryResource() noexcept = default;

        [[nodiscard]] StatusResult<MemoryBlock> Allocate(USize size, USize align) noexcept override {
            if (size == 0) {
                return StatusResult<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
            }
            if (!Alignment::IsPowerOfTwo(align)) {
                return StatusResult<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
            }
            void* p = ::operator new(static_cast<std::size_t>(size), std::nothrow);
            if (!p) { return StatusResult<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::Failed)); }
            return StatusResult<MemoryBlock>::Ok(MemoryBlock{ p, size });
        }

        void Deallocate(MemoryBlock block, USize align) noexcept override {
            if (block.Empty()) return;
            ::operator delete(block.ptr);
        }

        [[nodiscard]] StatusResult<MemoryBlock> Reallocate(MemoryBlock block, USize newSize, USize align) noexcept override {
            if (newSize == 0) {
                if (!block.Empty()) { ::operator delete(block.ptr, std::align_val_t(align)); }
                return StatusResult<MemoryBlock>::Ok(MemoryBlock{});
            }
            void* dst = ::operator new(static_cast<std::size_t>(newSize), std::nothrow);
            if (!dst) { return StatusResult<MemoryBlock>::Err(Err(StatusDomain::System(), StatusCode::Failed)); }
            auto* src = static_cast<const void*>(block.ptr);
            USize n = (block.size < newSize) ? block.size : newSize;
            MemMove(dst, src, n);
            if (!block.Empty()) { ::operator delete(block.ptr); }
            return StatusResult<MemoryBlock>::Ok(MemoryBlock{ dst, newSize });
        }

        [[nodiscard]] bool IsEqual(const IMemoryResource& other) const noexcept override {
            return reinterpret_cast<const void*>(this) == reinterpret_cast<const void*>(&other);
        }
    };
}
