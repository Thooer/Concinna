export module Prm.Meta:Reflectable;
import Prm.Element;

export namespace Prm {
    template<typename T>
    concept Reflectable = requires { { T::GetTypeName() } -> SameAs<const char*>; { T::GetMemberCount() } -> SameAs<USize>; };
    template<Enum E>
    struct EnumMemberInfo { E value; const char* name; };
    template<typename E>
    struct DescribeEnum;
    template<typename E>
    concept ReflectableEnum = std::is_enum_v<E> && requires { typename DescribeEnum<E>; { DescribeEnum<E>::GetTypeName() } -> SameAs<const char*>; { DescribeEnum<E>::GetMemberCount() } -> SameAs<USize>; };
}

