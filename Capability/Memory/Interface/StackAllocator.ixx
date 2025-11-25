export module Memory:StackAllocator;

import Language;
import :Definitions;
import :IMemoryResource;

export namespace Memory {
    struct StackAllocatorResource : IMemoryResource {
        void* m_base{nullptr};
        USize m_capacity{0};
        USize m_offset{0};
        USize m_committed{0};

        explicit StackAllocatorResource(USize capacity) noexcept;
        ~StackAllocatorResource() noexcept;

        [[nodiscard]] Expect<MemoryBlock> Allocate(USize size, USize align) noexcept override;
        void Deallocate(MemoryBlock block, USize align) noexcept override;
        [[nodiscard]] Expect<MemoryBlock> Reallocate(MemoryBlock block, USize newSize, USize align) noexcept override;
        [[nodiscard]] bool IsEqual(const IMemoryResource& other) const noexcept override;

        [[nodiscard]] USize Offset() const noexcept { return m_offset; }
        void ResetTo(USize off) noexcept;
    };
}