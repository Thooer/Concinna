export module Reflection:MemberInfo;
import Element;
import :TypeID;
import :TypeInfo;
import <concepts>;

export template<typename T>
struct MemberInfo {
    const char* name;
    USize offsetBytes;
    using Type = T;
    static consteval TypeID GetMemberTypeID() noexcept {
        if constexpr (requires { { T::StableName() } -> std::same_as<const char*>; }) {
            return GetTypeID<T>();
        } else {
            return TypeID{ SignatureHasherDefault::Hash(TypeInfo<T>::GetTypeName()) };
        }
    }
    static consteval USize GetMemberSizeBytes() noexcept { return sizeof(T); }
};
