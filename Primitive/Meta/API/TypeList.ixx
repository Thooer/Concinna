export module Prm.Meta:TypeList;
import Prm.Element;
import <type_traits>;

export namespace Prm {
    template<typename... Ts>
    struct TypeList { };

    template<typename T, typename List>
    struct Contains;
    template<typename T, typename... Ts>
    struct Contains<T, TypeList<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)> { };
    template<typename T, typename List>
    inline constexpr bool ContainsV = Contains<T, List>::value;

    template<typename T, typename List>
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
    template<typename T, typename List>
    inline constexpr int IndexOfV = IndexOf<T, List>::value;

    template<USize I, typename List>
    struct At;
    template<USize I, typename T, typename... Ts>
    struct At<I, TypeList<T, Ts...>> { using Type = typename At<I - 1, TypeList<Ts...>>::Type; };
    template<typename T, typename... Ts>
    struct At<0, TypeList<T, Ts...>> { using Type = T; };
    template<USize I, typename List>
    using AtT = typename At<I, List>::Type;

    template<typename L1, typename L2>
    struct Concat;
    template<typename... T1, typename... T2>
    struct Concat<TypeList<T1...>, TypeList<T2...>> { using Type = TypeList<T1..., T2...>; };
    template<typename L1, typename L2>
    using ConcatT = typename Concat<L1, L2>::Type;

    template<typename T, typename List>
    struct Remove;
    template<typename T, typename... Ts>
    struct Remove<T, TypeList<Ts...>> {
        template<typename U>
        struct NotSame : std::bool_constant<!std::is_same_v<T, U>> {};
        using Type = TypeList<std::conditional_t<NotSame<Ts>::value, Ts, void>...>;
    };
}

