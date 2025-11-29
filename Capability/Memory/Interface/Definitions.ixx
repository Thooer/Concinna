export module Cap.Memory:Definitions;

import Lang;
import Prm.Ownership;
import Prm.Sync;

export namespace Cap {
    struct MemoryBlock {
        void* ptr{nullptr};
        USize size{0};
        constexpr MemoryBlock() noexcept = default;
        constexpr MemoryBlock(void* p, USize s) noexcept : ptr(p), size(s) {}
        [[nodiscard]] constexpr bool Empty() const noexcept { return ptr == nullptr || size == 0; }
    };
    struct Alignment {
        static constexpr USize Default = 16;
        static constexpr USize CacheLine = 64;
        [[nodiscard]] static constexpr bool IsPowerOfTwo(USize value) noexcept { return value > 0 && (value & (value - 1)) == 0; }
        [[nodiscard]] static constexpr USize AlignUp(USize value, USize alignment) noexcept { return (value + (alignment - 1)) & ~(alignment - 1); }
    };
    template<typename T>
    using Atomic      = Prm::Atomic<T>;
    using MemoryOrder = Prm::MemoryOrder;
    using Backoff     = Prm::Backoff;
    using Prm::ThreadFence;
}
