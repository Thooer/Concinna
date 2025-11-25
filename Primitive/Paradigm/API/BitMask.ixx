
module;
export module Prm.Paradigm:BitMask;

import Prm.Element;

import <type_traits>;
import <utility>;
import <cstddef>;
import <cstdint>;
import <cstring>;
import <bit>;
import <new>;
 

export namespace Prm {
    export inline constexpr unsigned Bit(unsigned x) noexcept { return static_cast<unsigned>(1) << x; }


    // 位掩码枚举运算符启用钩子与实现
    template<typename Enum>
    struct EnableBitmaskOperators : std::false_type {};

    template<typename Enum>
    requires std::is_enum_v<Enum> && EnableBitmaskOperators<Enum>::value
    [[nodiscard]] constexpr Enum operator|(Enum lhs, Enum rhs) noexcept {
        using UT = std::underlying_type_t<Enum>;
        return static_cast<Enum>(static_cast<UT>(lhs) | static_cast<UT>(rhs));
    }

    template<typename Enum>
    requires std::is_enum_v<Enum> && EnableBitmaskOperators<Enum>::value
    [[nodiscard]] constexpr Enum operator&(Enum lhs, Enum rhs) noexcept {
        using UT = std::underlying_type_t<Enum>;
        return static_cast<Enum>(static_cast<UT>(lhs) & static_cast<UT>(rhs));
    }

    template<typename Enum>
    requires std::is_enum_v<Enum> && EnableBitmaskOperators<Enum>::value
    [[nodiscard]] constexpr Enum operator^(Enum lhs, Enum rhs) noexcept {
        using UT = std::underlying_type_t<Enum>;
        return static_cast<Enum>(static_cast<UT>(lhs) ^ static_cast<UT>(rhs));
    }

    template<typename Enum>
    requires std::is_enum_v<Enum> && EnableBitmaskOperators<Enum>::value
    [[nodiscard]] constexpr Enum operator~(Enum e) noexcept {
        using UT = std::underlying_type_t<Enum>;
        return static_cast<Enum>(~static_cast<UT>(e));
    }

    template<typename Enum>
    requires std::is_enum_v<Enum> && EnableBitmaskOperators<Enum>::value
    constexpr Enum& operator|=(Enum& lhs, Enum rhs) noexcept { return lhs = lhs | rhs; }

    template<typename Enum>
    requires std::is_enum_v<Enum> && EnableBitmaskOperators<Enum>::value
    constexpr Enum& operator&=(Enum& lhs, Enum rhs) noexcept { return lhs = lhs & rhs; }

    template<typename Enum>
    requires std::is_enum_v<Enum> && EnableBitmaskOperators<Enum>::value
    constexpr Enum& operator^=(Enum& lhs, Enum rhs) noexcept { return lhs = lhs ^ rhs; }

    template<typename Enum>
    requires std::is_enum_v<Enum> && EnableBitmaskOperators<Enum>::value
    [[nodiscard]] constexpr bool HasAny(Enum value, Enum flags) noexcept { return (value & flags) != static_cast<Enum>(0); }

    template<typename Enum>
    requires std::is_enum_v<Enum> && EnableBitmaskOperators<Enum>::value
    [[nodiscard]] constexpr bool HasAll(Enum value, Enum flags) noexcept { return (value & flags) == flags; }



    
    template<typename E>
    requires Enum
    struct Flags {
        using UT = std::underlying_type_t<E>;
        UT m_bits{};
        struct EndSentinel {};
        struct Iterator {
            static constexpr int kBits = static_cast<int>(sizeof(UT) * 8);
            UT bits;
            int idx;
            constexpr Iterator(UT b) noexcept : bits(b), idx(0) { advance(); }
            constexpr Iterator(UT b, int i) noexcept : bits(b), idx(i) {}
            constexpr void advance() noexcept {
                while (idx < kBits && ((bits & (static_cast<UT>(1) << idx)) == static_cast<UT>(0))) { ++idx; }
            }
            [[nodiscard]] constexpr E operator*() const noexcept { return static_cast<E>(static_cast<UT>(1) << idx); }
            constexpr Iterator& operator++() noexcept { ++idx; advance(); return *this; }
            friend constexpr bool operator!=(const Iterator& it, EndSentinel) noexcept { return it.idx < kBits; }
        };
        constexpr Flags() noexcept = default;
        constexpr explicit Flags(E e) noexcept : m_bits(static_cast<UT>(e)) {}
        [[nodiscard]] constexpr bool Test(E f) const noexcept { return (m_bits & static_cast<UT>(f)) != static_cast<UT>(0); }
        constexpr void Set(E f) noexcept { m_bits |= static_cast<UT>(f); }
        constexpr void Unset(E f) noexcept { m_bits &= ~static_cast<UT>(f); }
        [[nodiscard]] constexpr UT Value() const noexcept { return m_bits; }
        [[nodiscard]] constexpr bool Empty() const noexcept { return m_bits == static_cast<UT>(0); }
        [[nodiscard]] constexpr Iterator begin() const noexcept { return Iterator{ m_bits }; }
        [[nodiscard]] constexpr EndSentinel end() const noexcept { return EndSentinel{}; }
    };

}
