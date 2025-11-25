export module Prm.Meta:Meta;
import <type_traits>;
import <functional>;
import <cstddef>;
import <utility>;
import <tuple>;
import Prm.Element;
export import :CPO;

export namespace Prm {

    namespace detail {
        template<typename T>
        consteval const char* GetTypeSignature() noexcept {
        #if defined(_MSC_VER)
            return __FUNCSIG__;
        #elif defined(__clang__) || defined(__GNUC__)
            return __PRETTY_FUNCTION__;
        #else
            return "unknown";
        #endif
        }
    }

    // ------ TypeID ------
    struct TypeID {
        UInt64 value{};
        [[nodiscard]] constexpr bool operator==(const TypeID& rhs) const noexcept { return value == rhs.value; }
        [[nodiscard]] constexpr bool operator!=(const TypeID& rhs) const noexcept { return value != rhs.value; }
    };

    constexpr UInt64 SimpleStringHash(const char* s) noexcept {
        UInt64 hash = 5381;
        for (const char* p = s; *p; ++p) { hash = ((hash << 5) + hash) + static_cast<UInt64>(*p); }
        return hash;
    }
    struct SignatureHasherDefault {
        static consteval UInt64 Hash(const char* s) noexcept { return SimpleStringHash(s); }
    };

    template<typename T, typename Hasher = SignatureHasherDefault>
    consteval TypeID GetTypeID() noexcept {
        if constexpr (requires { { T::StableName() } -> SameAs<const char*>; }) {
            return TypeID{ Hasher::Hash(T::StableName()) };
        } else {
            static_assert(sizeof(T) == 0, "Provide stable type name via T::StableName().");
        }
    }

    // ------ TypeTag ------
    template<typename T>
    struct TypeTag { using Type = T; };

    template<typename T>
    inline constexpr TypeTag<T> TypeTagV{};

    // ------ TypeList ------
    template<typename... Ts>
    struct TypeList { };

    // Contains<T, TypeList<...>>
    template<typename T, typename List>
    struct Contains;

    template<typename T, typename... Ts>
    struct Contains<T, TypeList<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)> { };

    template<typename T, typename List>
    inline constexpr bool ContainsV = Contains<T, List>::value;

    // IndexOf<T, TypeList<...>>，找不到则返回 -1
    template<typename T, typename List>
    struct IndexOf;

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
        static constexpr int value = Impl<0, Ts...>::value;
    };

    template<typename T, typename List>
    inline constexpr int IndexOfV = IndexOf<T, List>::value;

    // At<I, TypeList<...>> -> 第 I 个类型
    template<USize I, typename List>
    struct At;
    template<USize I, typename T, typename... Ts>
    struct At<I, TypeList<T, Ts...>> { using Type = typename At<I - 1, TypeList<Ts...>>::Type; };
    template<typename T, typename... Ts>
    struct At<0, TypeList<T, Ts...>> { using Type = T; };
    template<USize I, typename List>
    using AtT = typename At<I, List>::Type;

    // Concat<TypeList<...>, TypeList<...>> -> TypeList<...>
    template<typename L1, typename L2>
    struct Concat;
    template<typename... T1, typename... T2>
    struct Concat<TypeList<T1...>, TypeList<T2...>> { using Type = TypeList<T1..., T2...>; };
    template<typename L1, typename L2>
    using ConcatT = typename Concat<L1, L2>::Type;

    // Remove<Type, TypeList<...>>
    template<typename T, typename List>
    struct Remove;
    template<typename T, typename... Ts>
    struct Remove<T, TypeList<Ts...>> {
        template<typename U>
        struct NotSame : std::bool_constant<!std::is_same_v<T, U>> {};
        using Type = TypeList<std::conditional_t<NotSame<Ts>::value, Ts, void>...>;
    };

    // 前置声明用于在 MemberInfo 使用，避免顺序问题
    template<typename T>
    struct TypeInfo;

    // ---------- Reflection core scaffolding ----------
    template<typename T>
    struct MemberInfo {
        const char* name;
        USize offsetBytes;
        using Type = T;
        static consteval TypeID GetMemberTypeID() noexcept {
            if constexpr (requires { { T::StableName() } -> SameAs<const char*>; }) {
                return GetTypeID<T>();
            } else {
                return TypeID{ SignatureHasherDefault::Hash(TypeInfo<T>::GetTypeName()) };
            }
        }
        static consteval USize GetMemberSizeBytes() noexcept { return sizeof(T); }
    };

    

    // TypeInfo模板定义
    template<typename T>
    struct TypeInfo {
        static consteval const char* GetTypeName() noexcept {
            return detail::GetTypeSignature<T>();
        }
    };

    template<typename... Components>
    struct TypeSet {
        using ComponentsList = TypeList<Components...>;
        static constexpr USize Count = sizeof...(Components);

        template<typename T>
        [[nodiscard]] static consteval bool Contains() noexcept {
            return ContainsV<T, ComponentsList>;
        }

        [[nodiscard]] friend constexpr bool operator==(const TypeSet&, const TypeSet&) noexcept { return true; }
    };

    

}

// ------ Callable traits (migrated from Traits) ------
export namespace Prm {
    template<typename T>
    struct CallableTraits;

    template<typename R, typename... Args>
    struct CallableTraits<R(Args...)> {
        using Return = R;
        using ArgsTuple = std::tuple<Args...>;
        static constexpr USize Arity = sizeof...(Args);
    };
    template<typename R, typename... Args>
    struct CallableTraits<R(Args...) noexcept> : CallableTraits<R(Args...)> {};

    template<typename R, typename... Args>
    struct CallableTraits<R(*)(Args...)> : CallableTraits<R(Args...)> {};
    template<typename R, typename... Args>
    struct CallableTraits<R(*)(Args...) noexcept> : CallableTraits<R(Args...)> {};
    template<typename R, typename... Args>
    struct CallableTraits<R(&)(Args...)> : CallableTraits<R(Args...)> {};

    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...)> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) const> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) &> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) const&> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) noexcept> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) const noexcept> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) volatile> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) const volatile> : CallableTraits<R(Args...)> {};

    template<typename F>
    struct CallableTraits : CallableTraits<decltype(&std::remove_cvref_t<F>::operator())> {};
}

export namespace Prm {
    template<typename T>
    concept Reflectable = requires {
        { T::GetTypeName() } -> SameAs<const char*>;
        { T::GetMemberCount() } -> SameAs<USize>;
    };

    template<Enum E>
    struct EnumMemberInfo {
        E value;
        const char* name;
    };

    template<typename E>
    struct DescribeEnum; // 主模板，由 REFLECTABLE_ENUM 宏特化

    template<typename E>
    concept ReflectableEnum = std::is_enum_v<E> && requires {
        typename DescribeEnum<E>;
        { DescribeEnum<E>::GetTypeName() } -> SameAs<const char*>;
        { DescribeEnum<E>::GetMemberCount() } -> SameAs<USize>;
    };

    // --- 便捷的自由函数 API ---

    template<ReflectableEnum E>
    [[nodiscard]] consteval const char* GetEnumTypeName() noexcept {
        return DescribeEnum<E>::GetTypeName();
    }

    template<ReflectableEnum E>
    [[nodiscard]] consteval USize GetEnumMemberCount() noexcept {
        return DescribeEnum<E>::GetMemberCount();
    }

    template<ReflectableEnum E>
    [[nodiscard]] consteval auto GetEnumMembers() noexcept {
        return DescribeEnum<E>::GetMembers();
    }


    template<typename T>
        requires Reflectable<T>
    struct Describe {
        static consteval const char* GetTypeName() noexcept { return T::GetTypeName(); }
        static consteval USize GetMemberCount() noexcept { return T::GetMemberCount(); }
        static consteval TypeID GetTypeID() noexcept { return ::Prm::GetTypeID<T>(); }
        static consteval USize GetTypeSize() noexcept { return sizeof(T); }

        // Member-level compile-time accessors generated by REFLECTABLE_STRUCT
        using MemberInfoList = typename T::MemberInfoList;
        using MemberTypes = typename T::MemberTypes;

        template<USize I>
        static consteval auto GetMemberInfo() noexcept { return T::template GetMemberInfo<I>(); }

        template<USize I>
        static consteval const char* GetMemberName() noexcept { return GetMemberInfo<I>().name; }
        template<USize I>
        static consteval USize GetMemberOffset() noexcept { return GetMemberInfo<I>().offset; }
        template<USize I>
        static consteval TypeID GetMemberTypeID() noexcept { return decltype(GetMemberInfo<I>())::GetMemberTypeID(); }
        template<USize I>
        static consteval USize GetMemberSize() noexcept { return decltype(GetMemberInfo<I>())::GetMemberSize(); }
    };

    template<typename T>
        requires Reflectable<T>
    consteval bool ValidateReflectionMetadata() noexcept {
        static_assert(Describe<T>::GetTypeName() != nullptr, "Type name cannot be null");
        static_assert(Describe<T>::GetTypeSize() > 0, "Type size must be positive");
        static_assert(Describe<T>::GetMemberCount() >= 0, "Member count cannot be negative");
        return true;
    }

    // --- 枚举底层互转 ---
    // ToUnderlying 已在 Core:Fundersmentals 中提供，这里仅提供 FromUnderlying。
    template<Enum E>
    using UnderlyingTypeT = std::underlying_type_t<E>;
    template<Enum E>
    [[nodiscard]] constexpr E FromUnderlying(UnderlyingTypeT<E> v) noexcept {
        return static_cast<E>(v);
    }

    // --- 成员遍历（聚合/反射结构） ---
    template<typename T, typename F>
        requires Reflectable<T>
    consteval void ForEachMember(F&& f) noexcept {
        constexpr USize count = Describe<T>::GetMemberCount();
        [&]<USize... Is>(std::index_sequence<Is...>) {
            (f(Describe<T>::template GetMemberInfo<Is>()), ...);
        }(std::make_index_sequence<count>{});
    }

    // --- Helper: get const member reference by index ---
    template<typename T, USize I>
        requires Reflectable<T>
    [[nodiscard]] constexpr decltype(auto) GetMemberRef(const T& v) noexcept {
        constexpr auto info = Describe<T>::template GetMemberInfo<I>();
        using M = typename decltype(info)::Type;
        const char* base = reinterpret_cast<const char*>(&v);
        const USize off = info.offset;
        return *reinterpret_cast<const M*>(base + off);
    }

    // --- Helper: projection functor for Meta.Generators ---
    // Provides a simple member accessor usable with HashGenerator/FormatGenerator/EqualGenerator.
    template<typename T, USize I>
        requires Reflectable<T>
    struct MemberGetter {
        static constexpr USize kIndex = I;
        [[nodiscard]] constexpr decltype(auto) operator()(const T& v) const noexcept {
            return GetMemberRef<T, I>(v);
        }
    };


    template<typename T>
        requires Reflectable<T>
    [[nodiscard]] constexpr USize tag_invoke(hash_t, const T& v) noexcept {
        auto mix = [](USize seed, USize h) constexpr noexcept -> USize {
            // boost::hash_combine style mixing (platform-stable on USize)
            return seed ^ (h + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2));
        };

        constexpr USize count = Describe<T>::GetMemberCount();
        return [&]<USize... Is>(std::index_sequence<Is...>) constexpr noexcept -> USize {
            USize seed = 0;
            ((seed = mix(seed, hash_t{}(GetMemberRef<T, Is>(v)))), ...);
            return seed;
        }(std::make_index_sequence<count>{});
    }

    template<typename T>
        requires Reflectable<T>
    [[nodiscard]] constexpr bool tag_invoke(equals_t, const T& a, const T& b) noexcept {
        constexpr USize count = Describe<T>::GetMemberCount();
        bool eq = true;
        [&]<USize... Is>(std::index_sequence<Is...>) constexpr noexcept {
            ((eq = eq && equals(GetMemberRef<T, Is>(a), GetMemberRef<T, Is>(b))), ...);
        }(std::make_index_sequence<count>{});
        return eq;
    }

    template<typename T, typename Visitor>
        requires Reflectable<T>
    constexpr void tag_invoke(visit_fields_t, Visitor&& visitor, T& obj) noexcept {
        constexpr USize count = Describe<T>::GetMemberCount();
        [&]<USize... Is>(std::index_sequence<Is...>) {
            (([&]{ auto info = Describe<T>::template GetMemberInfo<Is>();
                   using M = typename decltype(info)::Type;
                   auto* base = reinterpret_cast<unsigned char*>(&obj);
                   M& member = *reinterpret_cast<M*>(base + info.offsetBytes); visitor(info.name, member);}()), ...);
        }(std::make_index_sequence<count>{});
    }

    template<typename T>
        requires Reflectable<T>
    [[nodiscard]] constexpr USize tag_invoke(size_bytes_t, const T&) noexcept { return sizeof(T); }
}
