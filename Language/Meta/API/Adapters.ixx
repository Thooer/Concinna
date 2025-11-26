export module Meta:Adapters;
import Element;
import Reflection;
import :CPO;
import <utility>;


export template<typename Info>
using MemberTypeT = typename Info::Type;
export template<typename T>
struct HashVisitor {
    const T& v;
    USize& seed;
    static constexpr USize Mix(USize s, USize h) noexcept { return s ^ (h + 0x9e3779b97f4a7c15ull + (s << 6) + (s >> 2)); }
    template<typename Info>
    void operator()(Info info) const noexcept {
        using M = MemberTypeT<Info>;
        const auto* base = reinterpret_cast<const unsigned char*>(&v);
        const M& member = *reinterpret_cast<const M*>(base + info.offsetBytes);
        seed = Mix(seed, hash_t{}(member));
    }
};
export template<typename T>
struct EqualsVisitor {
    const T& a;
    const T& b;
    bool& eq;
    template<typename Info>
    void operator()(Info info) const noexcept {
        using M = MemberTypeT<Info>;
        const auto* ba = reinterpret_cast<const unsigned char*>(&a);
        const auto* bb = reinterpret_cast<const unsigned char*>(&b);
        const M& ma = *reinterpret_cast<const M*>(ba + info.offsetBytes);
        const M& mb = *reinterpret_cast<const M*>(bb + info.offsetBytes);
        eq = eq && equals(ma, mb);
    }
};
export template<typename T, typename Visitor>
struct VisitWrapper {
    Visitor& v;
    T& obj;
    template<typename Info>
    void operator()(Info info) const noexcept {
        using M = MemberTypeT<Info>;
        auto* base = reinterpret_cast<unsigned char*>(&obj);
        M& member = *reinterpret_cast<M*>(base + info.offsetBytes);
        v(info.name, member);
    }
};
export template<typename T>
    requires Reflectable<T>
[[nodiscard]] constexpr USize HashMembers(const T& v) noexcept {
    USize seed = 0;
    HashVisitor<T> vis{v, seed};
    ForEachMember<T>(vis);
    return seed;
}
export template<typename T>
    requires Reflectable<T>
[[nodiscard]] constexpr bool EqualsMembers(const T& a, const T& b) noexcept {
    bool eq = true;
    EqualsVisitor<T> vis{a, b, eq};
    ForEachMember<T>(vis);
    return eq;
}
export template<typename T, typename Visitor>
    requires Reflectable<T>
constexpr void VisitMembersImpl(Visitor&& visitor, T& obj) noexcept {
    VisitWrapper<T, Visitor> wrap{visitor, obj};
    ForEachMember<T>(wrap);
}
export template<typename T>
    requires Reflectable<T>
[[nodiscard]] constexpr USize tag_invoke(hash_t, const T& v) noexcept {
    return HashMembers<T>(v);
}
export template<typename T>
    requires Reflectable<T>
[[nodiscard]] constexpr bool tag_invoke(equals_t, const T& a, const T& b) noexcept {
    return EqualsMembers<T>(a, b);
}
export template<typename T, typename Visitor>
    requires Reflectable<T>
constexpr void tag_invoke(visit_fields_t, Visitor&& visitor, T& obj) noexcept {
    VisitMembersImpl<T, Visitor>(std::forward<Visitor>(visitor), obj);
}
export template<typename T>
    requires Reflectable<T>
[[nodiscard]] constexpr USize tag_invoke(size_bytes_t, const T&) noexcept { return sizeof(T); }
