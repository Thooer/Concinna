export module Foundation.IRSystem:Execution.Validator.Validator;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.Core.ObjectDescriptor;
import Foundation.IRSystem:IR.IR.MorphismIRValidator;
import Foundation.IRSystem:IR.IR.OpCode;

export namespace Foundation::IRSystem {

bool validate_morphism_ir_against_object(const MorphismIR& ir, const ObjectDescriptor& od) {
    if (!validate_morphism_ir(ir)) return false;
    for (const auto& b : ir.blocks) {
        for (const auto& n : b.nodes) {
            switch (n.opcode) {
                case OpCode::LoadMember: {
                    if (n.operands.load.field >= od.fields.size()) return false;
                    break;
                }
                case OpCode::StoreMember: {
                    if (n.operands.store.field >= od.fields.size()) return false;
                    break;
                }
                default: break;
            }
        }
    }
    return true;
}

}