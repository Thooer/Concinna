export module Meta:CPO;

import Element;
import :TagInvoke;

import <type_traits>;
import <limits>;
import <compare>;
import <functional>;

export struct hash_t {
    template<typename T>
    constexpr auto operator()(T&& value) const noexcept -> uint64_t { return tag_invoke(*this, std::forward<T>(value)); }
};
export inline constexpr hash_t hash{};

export struct Unavailable {
    template<typename... Args>
    constexpr void operator()(Args&&...) const = delete;
};

export struct equals_t {
    template<typename T>
    [[nodiscard]] constexpr bool operator()(const T& a, const T& b) const noexcept { return TagInvoke(*this, a, b); }
};
export inline constexpr equals_t equals{};

export struct compare_t {
    template<typename T>
    [[nodiscard]] constexpr Int32 operator()(const T& a, const T& b) const noexcept { return TagInvoke(*this, a, b); }
};
export inline constexpr compare_t compare{};

export struct visit_fields_t {
    template<typename Visitor, typename T>
    constexpr void operator()(Visitor&& v, T& obj) const noexcept { TagInvoke(*this, std::forward<Visitor>(v), obj); }
};
export inline constexpr visit_fields_t visit_fields{};

export struct size_bytes_t {
    template<typename T>
    [[nodiscard]] constexpr USize operator()(const T& v) const noexcept { return TagInvoke(*this, v); }
};
export inline constexpr size_bytes_t size_bytes{};

export template<typename T>
concept CpoHashable = requires(T&& value) { { hash(std::forward<T>(value)) } -> std::convertible_to<uint64_t>; };

export template<typename T>
concept CpoNothrowHashable = CpoHashable<T> && noexcept(hash(std::declval<T>()));

export template<typename T>
concept CpoComparable = TagInvocable<decltype(equals), const T&, const T&> && requires(const T& a, const T& b) { { TagInvoke(equals, a, b) } -> SameAs<bool>; };

export template<typename T>
concept CpoNothrowComparable = CpoComparable<T> && NothrowTagInvocable<decltype(equals), const T&, const T&>;
