export module Foundation.IRSystem:Execution.Optimizer.PassManager;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:Execution.Optimizer.ConstantPropagation;
import Foundation.IRSystem:Execution.Optimizer.SimplifyBranchCondConst;

export namespace Foundation::IRSystem {

export inline bool run_inplace(MorphismIR& ir, int opt_level = 0) {
    bool changed = false;
    changed = run_inplace_constant_propagation(ir) || changed;
    changed = run_inplace_simplify_branch_cond_const(ir) || changed;
    return changed;
}

}