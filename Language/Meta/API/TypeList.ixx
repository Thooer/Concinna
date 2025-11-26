export module Meta:TypeList;
import Element;
import <type_traits>;

export template<typename... Ts>
struct TypeList { };

export template<typename T, typename List>
struct Contains;
template<typename T, typename... Ts>
struct Contains<T, TypeList<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)> { };
export template<typename T, typename List>
inline constexpr bool ContainsV = Contains<T, List>::value;

export template<typename T, typename List>
struct IndexOf;
template<typename T, typename... Ts>
struct IndexOf<T, TypeList<Ts...>> {
private:
    static constexpr int kNotFound = -1;
    template<int Index, typename U, typename... Rest>
    struct Impl { static constexpr int value = [] { if constexpr (std::is_same_v<T, U>) return Index; else if constexpr (sizeof...(Rest) == 0) return kNotFound; else return Impl<Index + 1, Rest...>::value; }(); };
public:
    static constexpr int value = Impl<0, Ts...>::value;
};
export template<typename T, typename List>
inline constexpr int IndexOfV = IndexOf<T, List>::value;

export template<USize I, typename List>
struct At;
template<USize I, typename T, typename... Ts>
struct At<I, TypeList<T, Ts...>> { using Type = typename At<I - 1, TypeList<Ts...>>::Type; };
template<typename T, typename... Ts>
struct At<0, TypeList<T, Ts...>> { using Type = T; };
export template<USize I, typename List>
using AtT = typename At<I, List>::Type;

export template<typename L1, typename L2>
struct Concat;
template<typename... T1, typename... T2>
struct Concat<TypeList<T1...>, TypeList<T2...>> { using Type = TypeList<T1..., T2...>; };
export template<typename L1, typename L2>
using ConcatT = typename Concat<L1, L2>::Type;

export template<typename T, typename List>
struct Remove;
template<typename T, typename... Ts>
struct Remove<T, TypeList<Ts...>> {
    template<typename U>
    struct NotSame : std::bool_constant<!std::is_same_v<T, U>> {};
    using Type = TypeList<std::conditional_t<NotSame<Ts>::value, Ts, void>...>;
};
