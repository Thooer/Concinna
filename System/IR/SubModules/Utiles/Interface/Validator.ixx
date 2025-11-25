export module Foundation.IRSystem:Utiles.Validator;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.IR.MorphismIR;

export namespace Foundation::IRSystem {
export inline bool validate(const MorphismIR& ir) { return ir.entry_block < ir.blocks.size(); }
}