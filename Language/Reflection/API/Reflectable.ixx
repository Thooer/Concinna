export module Lang.Reflection:Reflectable;
import Lang.Element;
import <type_traits>;
import <concepts>;

template<typename T>
concept Reflectable = requires { { T::GetTypeName() } -> std::same_as<const char*>; { T::GetMemberCount() } -> std::same_as<USize>; };
template<typename E> requires std::is_enum_v<E>
struct EnumMemberInfo { E value; const char* name; };
template<typename E>
struct DescribeEnum;
template<typename E>
concept ReflectableEnum = std::is_enum_v<E> && requires { typename DescribeEnum<E>; { DescribeEnum<E>::GetTypeName() } -> std::same_as<const char*>; { DescribeEnum<E>::GetMemberCount() } -> std::same_as<USize>; };
