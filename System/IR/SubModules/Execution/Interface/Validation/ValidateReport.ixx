export module Foundation.IRSystem:Execution.Validation.ValidateReport;
import Lang;
import Foundation.IRSystem:IR.Core.Diagnostics;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.IR.MorphismIRMetadata;
import Foundation.IRSystem:IR.Core.ObjectDescriptor;
import Foundation.IRSystem:IR.IR.MorphismIRValidator;

export namespace Foundation::IRSystem::validation {

struct ValidateReport { bool ok; StaticVector<StaticString<256>, 64> messages; };

inline ValidateReport validate_report(const MorphismIR& ir, const MorphismIRMetadata& meta, const ObjectDescriptor& od) {
    ValidateReport r{ true, {} };
    {
        StaticString<256> msg{}; msg.append(StringView{"structural_invalid"});
        if (!validate_morphism_ir(ir)) { r.ok = false; (void)r.messages.push_back(msg); }
    }
    for (const auto& he : meta.hyperedges) {
        StringView cap{he.capability ? he.capability : ""};
        if (cap.size() == 0) { StaticString<256> msg{}; msg.append(StringView{"capability_tag_missing"}); r.ok = false; (void)r.messages.push_back(msg); break; }
    }
    for (const auto& b : ir.blocks) {
        for (const auto& n : b.nodes) {
            if (n.opcode == OpCode::LoadMember || n.opcode == OpCode::StoreMember) {
                if (n.opcode == OpCode::LoadMember && n.operands.load.field >= od.fields.size()) { StaticString<256> msg{}; msg.append(StringView{"field_index_out_of_range"}); r.ok = false; (void)r.messages.push_back(msg); }
                if (n.opcode == OpCode::StoreMember && n.operands.store.field >= od.fields.size()) { StaticString<256> msg{}; msg.append(StringView{"field_index_out_of_range"}); r.ok = false; (void)r.messages.push_back(msg); }
            }
        }
    }
    return r;
}

}