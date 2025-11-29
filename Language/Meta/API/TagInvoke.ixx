export module Lang.Meta:TagInvoke;
import Lang.Element;
import <utility>;

template<typename Tag, typename... Args>
concept TagInvocable = requires(Tag tag, Args... args) { tag_invoke(tag, args...); };

template<typename Tag, typename... Args>
concept NothrowTagInvocable = TagInvocable<Tag, Args...> && requires(Tag tag, Args... args) { { tag_invoke(tag, args...) } noexcept; };

template<typename Tag, typename... Args>
[[nodiscard]] constexpr auto TagInvoke(Tag&& tag, Args&&... args)
    noexcept(NothrowTagInvocable<Tag&&, Args&&...>) -> decltype(tag_invoke(std::forward<Tag>(tag), std::forward<Args>(args)...))
{
    return tag_invoke(std::forward<Tag>(tag), std::forward<Args>(args)...);
}

export template<typename Fn>
struct Cpo {
    [[no_unique_address]] Fn fn;
    template<typename... Args>
        requires (TagInvocable<Cpo, Args...> || Invocable<Fn&, Args...>)
    [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const
        noexcept(noexcept(TagInvoke(*this, std::forward<Args>(args)...)) || noexcept(fn(std::forward<Args>(args)...)))
    {
        if constexpr (TagInvocable<Cpo, Args...>) {
            return TagInvoke(*this, std::forward<Args>(args)...);
        } else {
            return fn(std::forward<Args>(args)...);
        }
    }
};
