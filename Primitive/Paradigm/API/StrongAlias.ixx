export module Prm.Paradigm:StrongAlias;

import <cstdint>;
import <cstddef>;
import <type_traits>;
import <limits>;
import Prm.Element;
// forward-declare CPO tags to avoid partition cycle
export namespace Prm { struct hash_t; struct equals_t; struct compare_t; }
 

export namespace Prm {
 
    template<typename Derived, template<typename> class... Mixins>
    struct ComposeMixins : Mixins<Derived>... {};


    template<typename T, typename Tag, template<typename> class... Mixins>
    struct StrongAlias : ComposeMixins<StrongAlias<T, Tag, Mixins...>, Mixins...> {
        using Value = T;
        using AliasTag = Tag;
        T m_value{};
        constexpr StrongAlias() noexcept = default;
        constexpr explicit StrongAlias(const T& v) noexcept(NothrowCopyConstructible<T>) : m_value(v) {}
        constexpr explicit StrongAlias(T&& v) noexcept(NothrowMoveConstructible<T>) : m_value(static_cast<T&&>(v)) {}
        [[nodiscard]] constexpr T& Get() & noexcept { return m_value; }
        [[nodiscard]] constexpr const T& Get() const& noexcept { return m_value; }

        // 幻影类型支持：模板标签版本
        // 使用模板模板参数作为类型标签，实现更强的类型区分
        template<template<typename> class PhantomTag>
        using PhantomType = StrongAlias<T, PhantomTag<T>, Mixins...>;

        // 便捷创建幻影类型的辅助函数
        template<template<typename> class PhantomTag>
        static constexpr PhantomType<PhantomTag> Create(const T& value) noexcept {
            return PhantomType<PhantomTag>{value};
        }
    };

    template<typename D>
    struct AliasHash { friend constexpr USize tag_invoke(hash_t, const D& v) noexcept { return TagInvoke(hash_t{}, v.m_value); } };
    template<typename D>
    struct AliasEquals { friend constexpr bool tag_invoke(equals_t, const D& a, const D& b) noexcept { return TagInvoke(equals_t{}, a.m_value, b.m_value); } };
    template<typename D>
    struct AliasCompare { friend constexpr Int32 tag_invoke(compare_t, const D& a, const D& b) noexcept { return TagInvoke(compare_t{}, a.m_value, b.m_value); } };

    // 便捷别名：避免重复书写底层类型
    template<typename BaseType, template<typename> class PhantomTag>
    using PhantomAlias = StrongAlias<BaseType, PhantomTag<BaseType>>;

    template<typename T, template<typename> class PhantomTag, template<typename> class... Mixins>
    struct Phantom : StrongAlias<T, PhantomTag<T>, Mixins...> {
        using Base = T;
        using TagType = PhantomTag<T>;
        using Strong = StrongAlias<T, PhantomTag<T>, Mixins...>;
        using Strong::StrongAlias;
    };

    template<template<typename> class PhantomTag, typename T, template<typename> class... Mixins>
    using PhantomOf = Phantom<RemoveCvRef<T>, PhantomTag, Mixins...>;

    template<template<typename> class PhantomTag, typename T>
    [[nodiscard]] constexpr auto MakePhantom(T&& v) noexcept(IsNothrowConstructible<Phantom<RemoveCvRef<T>, PhantomTag>, T&&>) -> Phantom<RemoveCvRef<T>, PhantomTag> {
        return Phantom<RemoveCvRef<T>, PhantomTag>{ static_cast<T&&>(v) };
    }

}
