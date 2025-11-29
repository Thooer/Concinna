export module Cap.Memory:NumaArena;

import Lang;
import :Definitions;
import :IMemoryResource;

export namespace Cap {
    struct NumaArenaResource : IMemoryResource {
        void* m_base{nullptr};
        USize m_capacity{0};
        USize m_offset{0};
        USize m_committed{0};
        UInt32 m_numaNode{0};
        bool m_largePages{false};

        explicit NumaArenaResource(USize capacity, UInt32 numaNode, bool useLargePages) noexcept;
        ~NumaArenaResource() noexcept;

        [[nodiscard]] Expect<MemoryBlock> Allocate(USize size, USize align) noexcept override;
        void Deallocate(MemoryBlock block, USize align) noexcept override;
        [[nodiscard]] Expect<MemoryBlock> Reallocate(MemoryBlock block, USize newSize, USize align) noexcept override;
        [[nodiscard]] bool IsEqual(const IMemoryResource& other) const noexcept override;

        void Reset() noexcept override;
        [[nodiscard]] USize Offset() const noexcept { return m_offset; }
        void ResetTo(USize off) noexcept;
    };
}
