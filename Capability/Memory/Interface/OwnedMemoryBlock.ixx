export module Memory:OwnedMemoryBlock;

import Language;
import :Definitions;
import :Allocator;

export namespace Memory {
    struct OwnedMemoryBlock {
        Allocator alloc{};
        MemoryBlock block{};
        USize align{Alignment::Default};

        constexpr OwnedMemoryBlock() noexcept = default;
        constexpr OwnedMemoryBlock(Allocator a, MemoryBlock b, USize al) noexcept : alloc(a), block(b), align(al) {}

        OwnedMemoryBlock(const OwnedMemoryBlock&) = delete;
        OwnedMemoryBlock& operator=(const OwnedMemoryBlock&) = delete;

        OwnedMemoryBlock(OwnedMemoryBlock&& other) noexcept : alloc(other.alloc), block(other.block), align(other.align) {
            other.alloc = Allocator{};
            other.block = MemoryBlock{};
            other.align = Alignment::Default;
        }
        OwnedMemoryBlock& operator=(OwnedMemoryBlock&& other) noexcept {
            if (this == &other) return *this;
            Reset();
            alloc = other.alloc;
            block = other.block;
            align = other.align;
            other.alloc = Allocator{};
            other.block = MemoryBlock{};
            other.align = Alignment::Default;
            return *this;
        }

        ~OwnedMemoryBlock() noexcept { Reset(); }

        [[nodiscard]] MemoryBlock Get() const noexcept { return block; }

        [[nodiscard]] MemoryBlock Release() noexcept {
            MemoryBlock out = block;
            block = MemoryBlock{};
            return out;
        }

        void Reset() noexcept {
            if (!block.Empty()) { alloc.Free(block, align); }
            block = MemoryBlock{};
        }
    };
}