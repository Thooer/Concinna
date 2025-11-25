export module Prm.Meta:CPO;

import Prm.Element;

import <type_traits>;
import <limits>;
import <compare>;
import <functional>;

export namespace Prm {
    // Hash CPO definition
    struct hash_t {
        template<typename T>
        constexpr auto operator()(T&& value) const noexcept -> uint64_t {
            return tag_invoke(*this, std::forward<T>(value));
        }
    };
    
    inline constexpr hash_t hash{};
    
    struct Unavailable {
        template<typename... Args>
        constexpr void operator()(Args&&...) const = delete;
    };

    // ---- 基础 CPO 标签与包装器（含安全回退实现） ----

    // equals — 默认回退顺序：ADL -> PlainData 按字节比较 -> operator==
    struct equals_t {
        template<typename T>
        [[nodiscard]] constexpr bool operator()(const T& a, const T& b) const noexcept { return TagInvoke(*this, a, b); }
    };
    inline constexpr equals_t equals{};
    

    // compare — 默认回退：ADL -> operator<=> -> operator<
    struct compare_t {
        template<typename T>
        [[nodiscard]] constexpr Int32 operator()(const T& a, const T& b) const noexcept { return TagInvoke(*this, a, b); }
    };
    inline constexpr compare_t compare{};
    struct visit_fields_t {
        template<typename Visitor, typename T>
        constexpr void operator()(Visitor&& v, T& obj) const noexcept { TagInvoke(*this, std::forward<Visitor>(v), obj); }
    };
    inline constexpr visit_fields_t visit_fields{};
    struct size_bytes_t {
        template<typename T>
        [[nodiscard]] constexpr USize operator()(const T& v) const noexcept { return TagInvoke(*this, v); }
    };
    inline constexpr size_bytes_t size_bytes{};
    

    // Hash helpers - forward declare to avoid circular dependency
    template<typename T>
    concept CpoHashable = requires(T&& value) {
        { hash(std::forward<T>(value)) } -> std::convertible_to<uint64_t>;
    };

    template<typename T>
    concept CpoNothrowHashable = CpoHashable<T> && 
        noexcept(hash(std::declval<T>()));

    template<typename T>
    concept CpoComparable = TagInvocable<decltype(equals), const T&, const T&> &&
        requires(const T& a, const T& b) {
            { TagInvoke(equals, a, b) } -> SameAs<bool>;
        };

    template<typename T>
    concept CpoNothrowComparable = CpoComparable<T> &&
        NothrowTagInvocable<decltype(equals), const T&, const T&>;

}
