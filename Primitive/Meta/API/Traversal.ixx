export module Prm.Meta:Traversal;
import Prm.Element;
import :Describe;
import :Reflectable;
import <utility>;

export namespace Prm {
    template<typename T, typename F>
        requires Reflectable<T>
    consteval void ForEachMember(F&& f) noexcept {
        constexpr USize count = Describe<T>::GetMemberCount();
        [&]<USize... Is>(std::index_sequence<Is...>) 
        { (f(Describe<T>::template GetMemberInfo<Is>()), ...); }
        (std::make_index_sequence<count>{});
    }
    template<typename T, USize I>
        requires Reflectable<T>
    [[nodiscard]] constexpr decltype(auto) GetMemberRef(const T& v) noexcept {
        constexpr auto info = Describe<T>::template GetMemberInfo<I>();
        using M = typename decltype(info)::Type;
        const char* base = reinterpret_cast<const char*>(&v);
        const USize off = info.offset;
        return *reinterpret_cast<const M*>(base + off);
    }
    template<typename T, USize I>
        requires Reflectable<T>
    struct MemberGetter {
        static constexpr USize kIndex = I;
        [[nodiscard]] constexpr decltype(auto) operator()(const T& v) const noexcept { return GetMemberRef<T, I>(v); }
    };
}
