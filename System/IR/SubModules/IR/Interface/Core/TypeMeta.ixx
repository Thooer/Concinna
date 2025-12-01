export module Foundation.IRSystem:IR.Core.TypeMeta;
import Foundation.IRSystem:IR.Core.Types;
import <type_traits>;
import <cstddef>;
import <string_view>;
import Lang;

export namespace Foundation::IRSystem {
    template<typename T> struct TypeIdentity { using Type = T; };
    template<USize... Is> struct IndexSequence {};
    namespace detail {
        template<USize N, USize... Is>
        struct MakeIndexSequenceImpl : MakeIndexSequenceImpl<N - 1, N - 1, Is...> {};
        template<USize... Is>
        struct MakeIndexSequenceImpl<0, Is...> { using Type = IndexSequence<Is...>; };
    }
    template<USize N>
    using MakeIndexSequence = typename detail::MakeIndexSequenceImpl<N>::Type;

    template<typename... Ts> struct TypeList {};
    template<typename List> struct Size;
    template<typename... Ts> struct Size<TypeList<Ts...>> : std::integral_constant<USize, sizeof...(Ts)> {};
    template<typename List> inline constexpr USize SizeV = Size<List>::value;
    template<USize I, typename List> struct At;
    template<USize I, typename H, typename... T> struct At<I, TypeList<H, T...>> : At<I - 1, TypeList<T...>> {};
    template<typename H, typename... T> struct At<0, TypeList<H, T...>> { using Type = H; };
    template<USize I, typename List> using AtT = typename At<I, List>::Type;

    template<typename L1, typename L2> struct Concat;
    template<typename... T1, typename... T2>
    struct Concat<TypeList<T1...>, TypeList<T2...>> { using Type = TypeList<T1..., T2...>; };
    template<typename L1, typename L2>
    using ConcatT = typename Concat<L1, L2>::Type;

    template<typename T, typename List> struct Remove;
    template<typename T, typename... Ts>
    struct Remove<T, TypeList<Ts...>> {
    private:
        template<typename... Us> static auto impl(TypeList<Us...>) -> TypeList<Us...>;
        template<typename U, typename... Rest>
        static auto filter(TypeList<U, Rest...>) {
            if constexpr (std::is_same_v<T, U>) {
                return filter(TypeList<Rest...>{});
            } else {
                using Next = decltype(filter(TypeList<Rest...>{}));
                return ConcatT<TypeList<U>, Next>{};
            }
        }
        static auto filter(TypeList<>) -> TypeList<>;
    public:
        using Type = decltype(filter(TypeList<Ts...>{}));
    };
    template<typename T, typename List>
    using RemoveT = typename Remove<T, List>::Type;

    template<typename T> using RemoveCvRef = std::remove_cv_t<std::remove_reference_t<T>>;
    template<typename T> std::add_rvalue_reference_t<T> Declval() noexcept;
    template <class...> using void_t = void;
    namespace detail {
        template <class D, class V, template<class...> class O, class... A>
        struct detector { using value_t = FalseType; using type = D; };
        template <class D, template<class...> class O, class... A>
        struct detector<D, void_t<O<A...>>, O, A...> { using value_t = TrueType; using type = O<A...>; };
    }
    template <template<class...> class O, class... A>
    using is_detected = typename detail::detector<void, void, O, A...>::value_t;

    namespace detail {
        consteval UInt64 FnV1a64(const std::string_view str) noexcept {
            // 使用Language中的FNV-1a哈希实现
            return FNV1aString(str);
        }
        template<typename T>
        consteval std::string_view GetTypeSignature() noexcept {
        #if defined(__clang__) || defined(__GNUC__)
            constexpr std::string_view signature = __PRETTY_FUNCTION__;
            constexpr std::string_view prefix = "[with T = ";
            constexpr std::string_view suffix = "]";
            const auto prefix_pos = signature.find(prefix);
            const auto suffix_pos = signature.find(suffix, prefix_pos);
            const auto start = prefix_pos + prefix.length();
            return signature.substr(start, suffix_pos - start);
        #elif defined(_MSC_VER)
            constexpr std::string_view signature = __FUNCSIG__;
            constexpr std::string_view prefix = "GetTypeSignature<";
            constexpr std::string_view suffix = ">(void)";
            const auto prefix_pos = signature.find(prefix);
            const auto suffix_pos = signature.find(suffix, prefix_pos);
            const auto start = prefix_pos + prefix.length();
            return signature.substr(start, suffix_pos - start);
        #else
            #error "GetTypeID not supported for this compiler"
        #endif
        }
    }

    struct TypeID { UInt64 value{}; constexpr bool operator==(const TypeID& rhs) const noexcept { return value == rhs.value; } constexpr bool operator!=(const TypeID& rhs) const noexcept { return value != rhs.value; } constexpr auto operator<=>(const TypeID& rhs) const noexcept = default; };
    template<typename T>
    consteval TypeID GetTypeID() noexcept { constexpr std::string_view type_name = detail::GetTypeSignature<RemoveCvRef<T>>(); return TypeID{ detail::FnV1a64(type_name) }; }
    template<typename T>
    consteval TypeID StableId() noexcept { return GetTypeID<RemoveCvRef<T>>(); }
    consteval UInt64 StableHash(std::string_view name) noexcept { return detail::FnV1a64(name); }
    template<typename T> struct TypeTag { using Type = T; };
    template<typename T> inline constexpr TypeTag<T> TypeTagV{};
    template<typename T, typename List> struct Contains;
    template<typename T, typename... Ts>
    struct Contains<T, TypeList<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)> { };
    template<typename T, typename List> inline constexpr bool ContainsV = Contains<T, List>::value;
    template<typename T, typename List> struct IndexOf;
    template<typename T, typename... Ts>
    struct IndexOf<T, TypeList<Ts...>> {
    private:
        static constexpr int kNotFound = -1;
        template<int Index, typename U, typename... Rest>
        struct Impl {
            static constexpr int value = [] {
                if constexpr (std::is_same_v<T, U>) return Index;
                else if constexpr (sizeof...(Rest) == 0) return kNotFound;
                else return Impl<Index + 1, Rest...>::value;
            }();
        };
    public:
        static constexpr int value = (sizeof...(Ts) > 0) ? Impl<0, Ts...>::value : kNotFound;
    };
    template<typename T, typename List> inline constexpr int IndexOfV = IndexOf<T, List>::value;
}