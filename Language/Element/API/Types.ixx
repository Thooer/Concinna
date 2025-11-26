export module Element:Types;
import :Concepts;
import <type_traits>;
import <utility>;
import <bit>;
import <cstdint>;
import <cstddef>;
import <type_traits>;
import <limits>;


export using UInt8  = std::uint8_t;
export using UInt16 = std::uint16_t;
export using UInt32 = std::uint32_t;
export using UInt64 = std::uint64_t;

export using Int8  = std::int8_t;
export using Int16 = std::int16_t;
export using Int32 = std::int32_t;
export using Int64 = std::int64_t;

export struct alignas(2) Half { UInt16 bits{}; };
export using Float32 = float;
export using Float64 = double;


export using Char8  = char8_t;
export using Char16 = char16_t;
export using Char32 = char32_t;

export using Byte   = std::uint8_t;
export using USize  = std::size_t;
export using SSize  = std::ptrdiff_t;
export using IntPtr  = std::intptr_t;
export using UIntPtr = std::uintptr_t;
export using TrueType = std::true_type;
export using FalseType = std::false_type;

export inline constexpr USize kDefaultAlignment = alignof(std::max_align_t);
export inline constexpr USize DynamicExtent = static_cast<USize>(-1);
export inline constexpr unsigned CACHELINE_SIZE = 64u;
