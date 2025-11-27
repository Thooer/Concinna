export module Cap.Memory:DebugMemoryResource;

import Language;
import :Definitions;
import :IMemoryResource;

export namespace Cap {
    export struct DebugMemoryResource : IMemoryResource {
        IMemoryResource* m_inner{nullptr};

        struct Entry { void* user; void* base; USize requested; USize total; USize align; Entry* next; };
        Entry* m_head{nullptr};
        Atomic<USize> m_count{0};

        explicit DebugMemoryResource(IMemoryResource* inner) noexcept;
        ~DebugMemoryResource() noexcept;

        [[nodiscard]] Expect<MemoryBlock> Allocate(USize size, USize align) noexcept override;
        void Deallocate(MemoryBlock block, USize align) noexcept override;
        [[nodiscard]] Expect<MemoryBlock> Reallocate(MemoryBlock block, USize newSize, USize align) noexcept override;
        [[nodiscard]] bool IsEqual(const IMemoryResource& other) const noexcept override;
    };
}
