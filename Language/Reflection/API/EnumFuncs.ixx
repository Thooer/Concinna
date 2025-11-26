export module Reflection:EnumFuncs;
import Element;
import :Reflectable;

export template<ReflectableEnum E>
[[nodiscard]] consteval const char* GetEnumTypeName() noexcept { return DescribeEnum<E>::GetTypeName(); }
export template<ReflectableEnum E>
[[nodiscard]] consteval USize GetEnumMemberCount() noexcept { return DescribeEnum<E>::GetMemberCount(); }
export template<ReflectableEnum E>
[[nodiscard]] consteval auto GetEnumMembers() noexcept { return DescribeEnum<E>::GetMembers(); }
