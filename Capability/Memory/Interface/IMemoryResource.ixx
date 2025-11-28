export module Cap.Memory:IMemoryResource;

import Language;
import :Definitions;

export namespace Cap {
    template<typename T>
    using StatusResult = Expect<T>;

    /// @brief 内存资源抽象基类 (ABI Stable)
    /// @details 定义了基于区域的内存管理原语。所有操作均为 noexcept。
    struct IMemoryResource {
        virtual ~IMemoryResource() = default;
        [[nodiscard]] virtual StatusResult<MemoryBlock> Allocate(USize size, USize align) noexcept = 0;
        virtual void Deallocate(MemoryBlock block, USize align) noexcept = 0;
        [[nodiscard]] virtual StatusResult<MemoryBlock> Reallocate(MemoryBlock block, USize newSize, USize align) noexcept = 0;
        [[nodiscard]] virtual bool IsEqual(const IMemoryResource& other) const noexcept = 0;
        [[nodiscard]] virtual bool IsOwnedByCurrentThread() const noexcept { return true; }
        virtual void Reset() noexcept {}
    };

    [[nodiscard]] inline bool operator==(const IMemoryResource& a, const IMemoryResource& b) noexcept {
        return a.IsEqual(b);
    }

    [[nodiscard]] inline bool operator!=(const IMemoryResource& a, const IMemoryResource& b) noexcept {
        return !(a == b);
    }
}
