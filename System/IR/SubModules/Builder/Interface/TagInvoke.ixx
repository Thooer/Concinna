#pragma once
import Foundation.IRSystem;

namespace Foundation::IRSystem {
    void tag_invoke();
    template<typename Tag, typename... Args>
    concept TagInvocable = requires { tag_invoke(Declval<Tag>(), Declval<Args>()...); };
    template<typename Tag, typename... Args>
    concept NothrowTagInvocable = TagInvocable<Tag, Args...> && requires { { tag_invoke(Declval<Tag>(), Declval<Args>()...) } noexcept; };
    template<typename Tag, typename... Args>
    [[nodiscard]] constexpr auto TagInvoke(Tag&& tag, Args&&... args)
        noexcept(NothrowTagInvocable<Tag, Args...>)
        -> decltype(tag_invoke(Forward<Tag>(tag), Forward<Args>(args)...))
    { return tag_invoke(Forward<Tag>(tag), Forward<Args>(args)...); }
    template<typename Fn>
    struct Cpo {
        Fn fn;
        template<typename... Args>
            requires TagInvocable<Cpo, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(NothrowTagInvocable<Cpo, Args...>)
        { return TagInvoke(*this, Forward<Args>(args)...); }
        template<typename... Args>
            requires (!TagInvocable<Cpo, Args...> && Invocable<Fn&, Args...>)
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(noexcept(fn(Forward<Args>(args)...)))
        { return fn(Forward<Args>(args)...); }
    };
}