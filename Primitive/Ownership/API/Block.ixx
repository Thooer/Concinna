export module Prm.Ownership:Block;
import Lang.Element;

export namespace Prm {
    struct MemoryBlock {
        void* ptr{nullptr};
        USize size{0};

        constexpr MemoryBlock() noexcept = default;
        constexpr MemoryBlock(void* p, USize s) noexcept : ptr(p), size(s) {}

        [[nodiscard]] constexpr bool Empty() const noexcept { return ptr == nullptr || size == 0; }
        [[nodiscard]] constexpr Byte* AsBytes() const noexcept { return static_cast<Byte*>(ptr); }

        [[nodiscard]] bool Contains(void* p) const noexcept {
            if (Empty()) return false;
            const UIntPtr start = reinterpret_cast<UIntPtr>(ptr);
            const UIntPtr target = reinterpret_cast<UIntPtr>(p);
            return target >= start && target < (start + size);
        }
    };

    struct Alignment {
        static constexpr USize Default = 16; // 与 Language 层默认保持一致
        static constexpr USize CacheLine = 64;

        [[nodiscard]] static constexpr bool IsPowerOfTwo(USize value) noexcept {
            return value > 0 && (value & (value - 1)) == 0;
        }

        [[nodiscard]] static constexpr USize AlignUp(USize value, USize alignment) noexcept {
            return (value + (alignment - 1)) & ~(alignment - 1);
        }
    };
}

