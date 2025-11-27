export module Cap.Memory:FrameAllocator;

import Language;
import :Definitions;
import :IMemoryResource;

export namespace Cap {
    struct FrameAllocatorResource : IMemoryResource {
        void* m_base{nullptr};
        USize m_capacity{0};
        USize m_offset{0};
        USize m_committed{0};

        explicit FrameAllocatorResource(USize capacity) noexcept;
        ~FrameAllocatorResource() noexcept;

        [[nodiscard]] Expect<MemoryBlock> Allocate(USize size, USize align) noexcept override;
        void Deallocate(MemoryBlock block, USize align) noexcept override;
        [[nodiscard]] Expect<MemoryBlock> Reallocate(MemoryBlock block, USize newSize, USize align) noexcept override;
        [[nodiscard]] bool IsEqual(const IMemoryResource& other) const noexcept override;

        void Reset() noexcept;
        [[nodiscard]] USize Offset() const noexcept { return m_offset; }
        void ResetTo(USize off) noexcept;
    };
}
