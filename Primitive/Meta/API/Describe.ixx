export module Prm.Meta:Describe;
import Prm.Element;
import :TypeID;
import :MemberInfo;
import :Reflectable;

export namespace Prm {
    template<typename T>
        requires Reflectable<T>
    struct Describe {
        static consteval const char* GetTypeName() noexcept { return T::GetTypeName(); }
        static consteval USize GetMemberCount() noexcept { return T::GetMemberCount(); }
        static consteval TypeID GetTypeID() noexcept { return ::Prm::GetTypeID<T>(); }
        static consteval USize GetTypeSize() noexcept { return sizeof(T); }
        using MemberInfoList = typename T::MemberInfoList;
        using MemberTypes = typename T::MemberTypes;
        template<USize I>
        static consteval auto GetMemberInfo() noexcept { return T::template GetMemberInfo<I>(); }
        template<USize I>
        static consteval const char* GetMemberName() noexcept { return GetMemberInfo<I>().name; }
        template<USize I>
        static consteval USize GetMemberOffset() noexcept { return GetMemberInfo<I>().offset; }
        template<USize I>
        static consteval TypeID GetMemberTypeID() noexcept { return decltype(GetMemberInfo<I>())::GetMemberTypeID(); }
        template<USize I>
        static consteval USize GetMemberSize() noexcept { return decltype(GetMemberInfo<I>())::GetMemberSize(); }
    };
    template<typename T>
        requires Reflectable<T>
    consteval bool ValidateReflectionMetadata() noexcept {
        static_assert(Describe<T>::GetTypeName() != nullptr, "Type name cannot be null");
        static_assert(Describe<T>::GetTypeSize() > 0, "Type size must be positive");
        static_assert(Describe<T>::GetMemberCount() >= 0, "Member count cannot be negative");
        return true;
    }
}

