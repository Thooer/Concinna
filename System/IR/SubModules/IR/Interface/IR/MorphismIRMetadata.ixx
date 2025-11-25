export module Foundation.IRSystem:IR.IR.MorphismIRMetadata;
import <vector>;
import <string_view>;
import <string>;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.Core.TypeMeta;
import Foundation.IRSystem:IR.IR.MorphismIR;

export namespace Foundation::IRSystem {
    struct MorphismIRMetadata {
        UInt32 version{1};
        std::string functor_name{};
        UInt64 two_cell_id{0};
        UInt64 naturality_square_id{0};
        std::vector<std::string_view> transform_chain;
        std::vector<std::string_view> pass_chain;
        struct LawTag { UInt64 id; const char* name; };
        std::vector<LawTag> applied_laws{};
        struct RoleIndex { TypeID role_id; UInt32 index; const char* source; };
        std::vector<RoleIndex> roles_to_indices{};
        struct SourceMap { TypeID formType; TypeID protocol_id; };
        std::vector<SourceMap> source_map{};
        std::vector<std::string_view> primitive_set{};
        struct HyperEdgeSource { UInt64 edge_id; std::vector<UInt64> nodes; const char* capability; };
        std::vector<HyperEdgeSource> hyperedges{};
        std::vector<std::string_view> effect_domains{};
        struct PassEffect { const char* name; UInt32 nodes_before; UInt32 nodes_after; UInt32 blocks_before; UInt32 blocks_after; };
        std::vector<PassEffect> pass_effects{};
        std::vector<std::string> errors{};
    };
}