export module Memory:SmallObjectAllocator;

import Language;
import :Definitions;
import :IMemoryResource;
import :PoolAllocator;

export namespace Memory {
    export struct SizeClassTable {
        static constexpr USize kClasses[16] = { 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256 };
        static constexpr USize kCount = 16;
        static constexpr USize kMaxSmall = kClasses[kCount - 1];

        [[nodiscard]] static constexpr USize ClassIndex(USize size) noexcept {
            for (USize i = 0; i < kCount; ++i) {
                if (size <= kClasses[i]) return i;
            }
            return kCount;
        }

        [[nodiscard]] static constexpr USize ClassSize(USize idx) noexcept {
            return idx < kCount ? kClasses[idx] : 0;
        }
    };

    export struct SmallObjectAllocatorResource : IMemoryResource {
        PoolAllocatorResource** m_pools{nullptr};
        USize m_poolCount{0};
        USize m_alignment{Alignment::Default};
        IMemoryResource* m_fallback{nullptr};

        explicit SmallObjectAllocatorResource(USize perPoolCapacity, IMemoryResource* fallback) noexcept;
        ~SmallObjectAllocatorResource() noexcept;

        [[nodiscard]] Expect<MemoryBlock> Allocate(USize size, USize align) noexcept override;
        void Deallocate(MemoryBlock block, USize align) noexcept override;
        [[nodiscard]] Expect<MemoryBlock> Reallocate(MemoryBlock block, USize newSize, USize align) noexcept override;
        [[nodiscard]] bool IsEqual(const IMemoryResource& other) const noexcept override;
    };
}