export module Foundation.IRSystem:Execution.Optimizer.SimplifyBranchCondConst;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.IR.OpCode;

export namespace Foundation::IRSystem {

export inline bool run_inplace_simplify_branch_cond_const(MorphismIR& ir) {
    bool changed = false;
    for (auto& bb : ir.blocks) {
        if (bb.terminator.opcode == OpCode::BranchCond) {
            const auto cond_reg = bb.terminator.operands.brcond.cond;
            bool found_local_def = false;
            for (const auto& n : bb.nodes) {
                if (n.result_reg == cond_reg && n.opcode == OpCode::Const) {
                    found_local_def = true;
                    const auto cid = n.operands.const_id;
                    if (cid < ir.constant_pool.size() && std::holds_alternative<bool>(ir.constant_pool[cid])) {
                        const bool v = std::get<bool>(ir.constant_pool[cid]);
                        bb.terminator.opcode = OpCode::Branch;
                        bb.terminator.operands.branch_target = v ? bb.terminator.operands.brcond.true_block : bb.terminator.operands.brcond.false_block;
                        changed = true;
                    }
                    break;
                }
            }
            (void)found_local_def;
        }
    }
    return changed;
}

}