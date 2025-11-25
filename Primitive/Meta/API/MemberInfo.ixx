export module Prm.Meta:MemberInfo;
import Prm.Element;
import :TypeID;
import :TypeInfo;

export namespace Prm {
    template<typename T>
    struct MemberInfo {
        const char* name;
        USize offsetBytes;
        using Type = T;
        static consteval TypeID GetMemberTypeID() noexcept {
            if constexpr (requires { { T::StableName() } -> SameAs<const char*>; }) {
                return GetTypeID<T>();
            } else {
                return TypeID{ SignatureHasherDefault::Hash(TypeInfo<T>::GetTypeName()) };
            }
        }
        static consteval USize GetMemberSizeBytes() noexcept { return sizeof(T); }
    };
}

