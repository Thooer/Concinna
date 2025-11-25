export module Prm.Meta:TypeSet;
import Prm.Element;
import :TypeList;

export namespace Prm {
    template<typename... Components>
    struct TypeSet {
        using ComponentsList = TypeList<Components...>;
        static constexpr USize Count = sizeof...(Components);
        template<typename T>
        static consteval bool Contains() noexcept { return ContainsV<T, ComponentsList>; }
        [[nodiscard]] friend constexpr bool operator==(const TypeSet&, const TypeSet&) noexcept { return true; }
    };
}

