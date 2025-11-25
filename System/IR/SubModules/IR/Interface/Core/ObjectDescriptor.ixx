export module Foundation.IRSystem:IR.Core.ObjectDescriptor;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.Core.TypeMeta;
import <span>;

export namespace Foundation::IRSystem {

using FieldIndex = UInt32;

struct FieldDescriptor { const char* name; TypeID fieldType; USize offsetBytes; };
struct RoleIndexDesc { TypeID roleId; FieldIndex index; const char* name; };

struct ObjectDescriptor {
    TypeID      formType;
    USize       sizeBytes;
    USize       alignBytes;
    UInt32      version = 1;
    std::span<const FieldDescriptor> fields;
    std::span<const TypeID> protocol_ids{};
    std::span<const RoleIndexDesc> roles{};
    [[nodiscard]] constexpr int find_field_index(TypeID field_type) const noexcept { for (int i = 0; i < fields.size(); ++i) { if (fields[i].fieldType == field_type) return i; } return -1; }
    [[nodiscard]] constexpr int find_role_index(TypeID role_id) const noexcept { for (int i = 0; i < roles.size(); ++i) { if (roles[i].roleId == role_id) return static_cast<int>(roles[i].index); } return -1; }
    [[nodiscard]] constexpr bool has_protocol(TypeID proto_id) const noexcept { for (const auto& pid : protocol_ids) { if (pid == proto_id) return true; } return false; }
};

}