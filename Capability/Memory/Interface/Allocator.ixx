export module Cap.Memory:Allocator;

import Lang;
import :Definitions;
import :IMemoryResource;

export namespace Cap {

    /// @brief 内存分配器句柄 (Handle)
    /// @details 轻量级对象，持有 IMemoryResource 的指针。
    struct Allocator {
        IMemoryResource* resource{nullptr};

        constexpr Allocator() noexcept = default;
        constexpr Allocator(IMemoryResource* r) noexcept : resource(r) {}
        
        [[nodiscard]] bool IsValid() const noexcept { return resource != nullptr; }

        // --- Raw Bytes Access ---

        [[nodiscard]] Expect<MemoryBlock> Alloc(USize size, USize align = Alignment::Default) const noexcept {
            // ASSERT(IsValid());
            return resource->Allocate(size, align);
        }

        void Free(MemoryBlock block, USize align = Alignment::Default) const noexcept {
            if (block.Empty()) return;
            // ASSERT(IsValid());
            resource->Deallocate(block, align);
        }

        // --- Object Lifecycle Helpers ---

        /// @brief 分配并构造对象 (Placement New)
        template<typename T, typename... Args>
        [[nodiscard]] Expect<T*> New(Args&&... args) const noexcept {
            Expect<MemoryBlock> block = Alloc(sizeof(T), alignof(T));
            if (!block.IsOk()) {
                return Expect<T*>::Err(block.Error());
            }

            void* ptr = block.Value().ptr;
            new (ptr) T(Forward<Args>(args)...);
            return Expect<T*>::Ok(static_cast<T*>(ptr));
        }

        /// @brief 析构并释放对象
        template<typename T>
        void Delete(T* ptr, USize size) const noexcept {
            if (!ptr) return;
            if constexpr (!TriviallyDestructible<T>) { ptr->~T(); }
            Free(MemoryBlock{ptr, size}, alignof(T));
        }

        [[nodiscard]] Expect<MemoryBlock> AllocZeroed(USize size, USize align = Alignment::Default) const noexcept {
            Expect<MemoryBlock> r = Alloc(size, align);
            if (!r.IsOk()) return r;
            auto blk = r.Value();
            auto* p = static_cast<Byte*>(blk.ptr);
            for (USize i = 0; i < blk.size; ++i) { p[i] = static_cast<Byte>(0); }
            return r;
        }

        [[nodiscard]] bool TryExpand(void* ptr, USize oldSize, USize delta, USize align = Alignment::Default) const noexcept {
            if (!ptr || !IsValid()) return false;
            const USize newSize = oldSize + delta;
            auto rr = resource->Reallocate(MemoryBlock{ ptr, oldSize }, newSize, align);
            if (!rr.IsOk()) return false;
            return rr.Value().ptr == ptr;
        }
    };
}
