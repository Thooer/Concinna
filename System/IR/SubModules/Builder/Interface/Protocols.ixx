#pragma once
import Foundation.IRSystem;
import <span>;

namespace Foundation::IRSystem {
    struct implemented_protocols_tag {
        template<typename TForm>
        constexpr auto operator()(const TForm&) const { return TypeList<>(); }
    };
    inline constexpr Cpo<implemented_protocols_tag> implemented_protocols{implemented_protocols_tag{}};
    template<typename TForm, typename TProtocol>
    concept Implements = ContainsV<TProtocol, decltype(implemented_protocols(Declval<TForm>()))>;
    struct ProtocolDescData { TypeID id; const char* name; std::span<const char*> role_names; };
    #define CATEGORY_REGISTER_PROTOCOL_DESC(PROTO_TAG, NAME_STR, ROLE_ARRAY) \
        inline const ::Foundation::IRSystem::ProtocolDescData PROTO_TAG##Desc { \
            .id = ::Foundation::IRSystem::GetTypeID<PROTO_TAG>(), \
            .name = NAME_STR, \
            .role_names = std::span<const char*>{ ROLE_ARRAY, sizeof(ROLE_ARRAY)/sizeof(ROLE_ARRAY[0]) } \
        }
}

namespace Foundation::IRSystem::protocols {
    struct NullableProtocol {};
    struct MaybeProtocol {};
    struct IterableProtocol {};
    struct ActorProtocol {};
    inline const char* NullableRoleNames[] = { "has_value", "value" };
    inline const char* MaybeRoleNames[] = { "has_value", "value" };
    inline const char* IterableRoleNames[] = { "data", "size", "index", "sum" };
    CATEGORY_REGISTER_PROTOCOL_DESC(NullableProtocol, "Nullable", NullableRoleNames);
    CATEGORY_REGISTER_PROTOCOL_DESC(MaybeProtocol,    "Maybe",    MaybeRoleNames);
    CATEGORY_REGISTER_PROTOCOL_DESC(IterableProtocol,  "Iterable", IterableRoleNames);
    inline const ::Foundation::IRSystem::ProtocolDescData ActorProtocolDesc{ .id = ::Foundation::IRSystem::GetTypeID<ActorProtocol>(), .name = "Actor", .role_names = std::span<const char*>{} };
}