export module Prm.Meta:EnumFuncs;
import Prm.Element;
import :Reflectable;

export namespace Prm {
    template<ReflectableEnum E>
    [[nodiscard]] consteval const char* GetEnumTypeName() noexcept { return DescribeEnum<E>::GetTypeName(); }
    template<ReflectableEnum E>
    [[nodiscard]] consteval USize GetEnumMemberCount() noexcept { return DescribeEnum<E>::GetMemberCount(); }
    template<ReflectableEnum E>
    [[nodiscard]] consteval auto GetEnumMembers() noexcept { return DescribeEnum<E>::GetMembers(); }
}

