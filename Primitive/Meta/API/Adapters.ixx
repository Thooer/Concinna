export module Prm.Meta:Adapters;
import Prm.Element;
import :Describe;
import :Traversal;
import :CPO;
import <utility>;

export namespace Prm {
    template<typename T>
        requires Reflectable<T>
    [[nodiscard]] constexpr USize tag_invoke(hash_t, const T& v) noexcept {
        auto mix = [](USize seed, USize h) constexpr noexcept -> USize { return seed ^ (h + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2)); };
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
        [&]<USize... Is>(std::index_sequence<Is...>) constexpr noexcept { ((eq = eq && equals(GetMemberRef<T, Is>(a), GetMemberRef<T, Is>(b))), ...); }(std::make_index_sequence<count>{});
        return eq;
    }
    template<typename T, typename Visitor>
        requires Reflectable<T>
    constexpr void tag_invoke(visit_fields_t, Visitor&& visitor, T& obj) noexcept {
        constexpr USize count = Describe<T>::GetMemberCount();
        [&]<USize... Is>(std::index_sequence<Is...>) {
            (([&]{ auto info = Describe<T>::template GetMemberInfo<Is>(); using M = typename decltype(info)::Type; auto* base = reinterpret_cast<unsigned char*>(&obj); M& member = *reinterpret_cast<M*>(base + info.offsetBytes); visitor(info.name, member);}()), ...);
        }(std::make_index_sequence<count>{});
    }
    template<typename T>
        requires Reflectable<T>
    [[nodiscard]] constexpr USize tag_invoke(size_bytes_t, const T&) noexcept { return sizeof(T); }
}
