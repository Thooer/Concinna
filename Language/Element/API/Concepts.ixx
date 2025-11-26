module;
#include <type_traits>
#include <concepts>
#include <limits>
#include <compare>
#include <utility>

export module Element:Concepts;

import :Traits;

export template<typename T>
concept Integral = std::is_integral_v<T>;

export template<typename T>
concept SignedIntegral = Integral<T> && std::is_signed_v<T>;

export template<typename T>
concept UnsignedIntegral = Integral<T> && std::is_unsigned_v<T>;

export template<typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

export template<typename T>
concept Arithmetic = Integral<T> || FloatingPoint<T>;

export template<typename T>
concept Enum = std::is_enum_v<T>;

// --- 对象核心属性概念 (Core Object Property Concepts) ---

export template<typename T>
concept Trivial = std::is_trivial_v<T>;

export template<typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

export template<typename T>
concept StandardLayout = std::is_standard_layout_v<T>;

export template<typename T>
concept Pod = Trivial<T> && StandardLayout<T>;

// --- 可操作性概念 (Operational Concepts) ---

export template<typename T>
concept DefaultConstructible = std::default_initializable<T>;

export template<typename T>
concept MoveConstructible = std::move_constructible<T>;

export template<typename T>
concept CopyConstructible = std::copy_constructible<T>;


export template<typename T>
concept MoveAssignable = std::assignable_from<T&, T&&>;

export template<typename T>
concept CopyAssignable = std::assignable_from<T&, const T&>;

export template<typename T>
concept Movable = std::movable<T>;

export template<typename T>
concept Copyable = std::copyable<T>;

// --- 类型关系与比较概念 (Type Relation & Comparison Concepts) ---

export template<typename T, typename U>
concept SameAs = std::same_as<T, U>;

export template<typename Derived, typename Base>
concept DerivedFrom = std::derived_from<Derived, Base>;

export template<typename From, typename To>
concept ConvertibleTo = std::convertible_to<From, To>;

export template<typename F, typename... Args>
concept Invocable = std::invocable<F, Args...>;

export template<typename F, typename... Args>
concept Predicate = std::predicate<F, Args...>;


// --- noexcept 保证概念 (Noexcept Guarantee Concepts) ---

export template<typename T>
concept NothrowMoveConstructible = std::is_nothrow_move_constructible_v<T>;

export template<typename T>
concept NothrowCopyConstructible = std::is_nothrow_copy_constructible_v<T>;


// 变参版本的构造不抛异常概念，覆盖所有参数组合
export template<typename T, typename... Args>
concept IsNothrowConstructible = std::is_nothrow_constructible_v<T, Args...>;


export template<typename T, typename U>
concept IsNothrowAssignable = std::is_nothrow_assignable_v<T, U>;

// --- 库专属高级语义概念 (Library-Specific High-Level Semantic Concepts) ---
// 移除库专属概念，保留语言级通用约束
export template<typename T>
concept Regular = std::regular<T>;

export template<typename T>
concept ThreeWayComparable = std::three_way_comparable<T>;

// --- 通用工具概念 (General Utility Concepts) ---


export template<typename T>
concept IsClass = std::is_class_v<T>;

export template<typename T>
concept IsTypeEmpty = std::is_empty_v<T>;

export template<typename T, typename... Args>
concept ConstructibleFrom = std::is_constructible_v<T, Args...>;

export template<typename T>
concept EqualityComparable = std::equality_comparable<T>;

export template<typename T>
concept HasSize = requires(const T& t) { { t.size() } -> std::convertible_to<std::size_t>; };

export template<typename T, typename... Ts>
concept IsAnyOf = (SameAs<T, Ts> || ...);




// --- 可调用族概念 (Callable Family Concepts) ---



// 保留基础可调用概念，不施加库专属安全约束

// 返回类型匹配（严格 SameAs）
export template<typename R, typename F, typename... Args>
concept CallableReturning = Invocable<F&, Args...> && SameAs<InvokeResult<F, Args...>, R>;

// noexcept 调用约束（不检查返回类型安全）
export template<typename F, typename... Args>
concept NothrowCallable = Invocable<F&, Args...> && requires(F& f, Args... args) {
    { f(std::declval<Args>()...) } noexcept;
};

export template<typename T>
concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

// ------------------------------
// Range 与 MonadicType 概念
// ------------------------------
// 轻量 Range 概念：要求存在 begin()/end()，不依赖 <ranges>
export template<typename R>
concept Range = requires(R r) {
    { r.begin() };
    { r.end() };
};

// 带 size() 的范围（与已有 HasSize 组合）
export template<typename R>
concept SizedRange = Range<R> && HasSize<R>;

// MonadicType 最小语义：具备 Has() 查询能力
// 可与 AndThen/Transform 等管道式操作搭配使用，但此处不做强制约束
export template<typename M>
concept MonadicType = requires(M m) {
    { m.Has() } -> SameAs<bool>;
};

export template<typename R>
concept ResultLike = requires(R r) {
    { r.IsOk() } -> SameAs<bool>;
    r.Value();
    r.Error();
};
