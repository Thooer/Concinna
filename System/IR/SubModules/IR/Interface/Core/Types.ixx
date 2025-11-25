export module Foundation.IRSystem:IR.Core.Types;
import <cstdint>;
import <cstddef>;
import <type_traits>;
import <limits>;

export namespace Foundation::IRSystem {
    using UInt8  = std::uint8_t;
    using UInt16 = std::uint16_t;
    using UInt32 = std::uint32_t;
    using UInt64 = std::uint64_t;

    using Int8  = std::int8_t;
    using Int16 = std::int16_t;
    using Int32 = std::int32_t;
    using Int64 = std::int64_t;

    using Float32 = float;
    using Float64 = double;

    using Char8  = char8_t;
    using Char16 = char16_t;
    using Char32 = char32_t;

    using Byte   = std::uint8_t;
    using USize  = std::size_t;
    using SSize  = std::ptrdiff_t;
    using IntPtr  = std::intptr_t;
    using UIntPtr = std::uintptr_t;
    using TrueType = std::true_type;
    using FalseType = std::false_type;

    struct NullOptT { explicit constexpr NullOptT(int) noexcept {} };
    inline constexpr NullOptT NullOpt{0};

    inline constexpr USize DynamicExtent = static_cast<USize>(-1);
}