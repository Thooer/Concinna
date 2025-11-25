export module Memory:Definitions;

import Language;

export namespace Memory {
    using Byte    = Byte;
    using USize   = USize;
    using UIntPtr = UIntPtr;

    /// @brief 表示一块已分配的内存区域
    /// @note 包含了指针和实际分配的大小（可能大于请求大小）
    struct MemoryBlock {
        void* ptr{nullptr};
        USize size{0};

        constexpr MemoryBlock() noexcept = default;
        constexpr MemoryBlock(void* p, USize s) noexcept : ptr(p), size(s) {}

        [[nodiscard]] constexpr bool Empty() const noexcept { return ptr == nullptr || size == 0; }
        [[nodiscard]] constexpr Byte* AsBytes() const noexcept { return static_cast<Byte*>(ptr); }
        
        /// @brief 检查块内是否包含某个地址
        [[nodiscard]] bool Contains(void* p) const noexcept {
            if (Empty()) return false;
            const UIntPtr start = reinterpret_cast<UIntPtr>(ptr);
            const UIntPtr target = reinterpret_cast<UIntPtr>(p);
            return target >= start && target < (start + size);
        }
    };

    /// @brief 内存对齐工具
    struct Alignment {
        static constexpr USize Default = kDefaultAlignment;
        static constexpr USize CacheLine = 64;

        [[nodiscard]] static constexpr bool IsPowerOfTwo(USize value) noexcept {
            return value > 0 && (value & (value - 1)) == 0;
        }

        [[nodiscard]] static constexpr USize AlignUp(USize value, USize alignment) noexcept {
            // ASSERT(IsPowerOfTwo(alignment));
            return (value + (alignment - 1)) & ~(alignment - 1);
        }
    };
}