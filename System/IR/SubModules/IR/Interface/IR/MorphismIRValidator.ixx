export module Foundation.IRSystem:IR.IR.MorphismIRValidator;
import <optional>;
import <vector>;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.IR.OpCode;

export namespace Foundation::IRSystem {

inline RegisterID compute_max_reg_for_validation(const MorphismIR& ir) {
    RegisterID max_reg = 0;
    for (const auto& blk : ir.blocks) {
        for (const auto& node : blk.nodes) {
            if (node.result_reg != static_cast<RegisterID>(-1)) {
                max_reg = std::max(max_reg, node.result_reg);
            }
        }
    }
    return max_reg;
}

inline bool validate_morphism_ir(const MorphismIR& ir) {
    if (ir.entry_block >= ir.blocks.size()) return false;
    const RegisterID max_reg = compute_max_reg_for_validation(ir);
    std::vector<bool> defined(static_cast<USize>(max_reg) + 1, false);
    for (const auto& bb : ir.blocks) {
        for (const auto& n : bb.nodes) {
            if (n.result_reg != static_cast<RegisterID>(-1) && static_cast<USize>(n.result_reg) < defined.size()) {
                defined[n.result_reg] = true;
            }
        }
    }
    std::vector<bool> reachable(ir.blocks.size(), false);
    std::vector<BlockID> queue; queue.reserve(ir.blocks.size());
    queue.push_back(ir.entry_block); reachable[ir.entry_block] = true;
    for (USize qi = 0; qi < queue.size(); ++qi) {
        const BlockID bid = queue[qi];
        const auto& bb = ir.blocks[bid];
        switch (bb.terminator.opcode) {
            case OpCode::Branch: {
                BlockID t = bb.terminator.operands.branch_target;
                if (t < ir.blocks.size() && !reachable[t]) { reachable[t] = true; queue.push_back(t); }
                break;
            }
            case OpCode::BranchCond: {
                BlockID t = bb.terminator.operands.brcond.true_block;
                BlockID f = bb.terminator.operands.brcond.false_block;
                if (t < ir.blocks.size() && !reachable[t]) { reachable[t] = true; queue.push_back(t); }
                if (f < ir.blocks.size() && !reachable[f]) { reachable[f] = true; queue.push_back(f); }
                break;
            }
            default: break;
        }
    }
    for (BlockID bid = 0; bid < ir.blocks.size(); ++bid) {
        const auto& bb = ir.blocks[bid];
        if (!reachable[bid]) return false;
        if (!bb.hasExplicitTerminator) return false;
        if (bb.terminator.result_reg != static_cast<RegisterID>(-1)) return false;
        for (USize idx = 0; idx < bb.nodes.size(); ++idx) {
            const auto& n = bb.nodes[idx];
            switch (n.opcode) {
                case OpCode::Arg: {
                    if (n.operands.arg_index >= ir.expected_args) return false;
                    break;
                }
                case OpCode::Call: {
                    const auto off = n.operands.call.args_offset;
                    const auto cnt = n.operands.call.args_count;
                    if (static_cast<USize>(off) + static_cast<USize>(cnt) > ir.call_arg_regs.size()) return false;
                    for (UInt32 i = 0; i < cnt; ++i) {
                        USize pos = static_cast<USize>(off) + static_cast<USize>(i);
                        RegisterID arg_reg = ir.call_arg_regs[pos];
                        if (arg_reg > max_reg || !defined[arg_reg]) return false;
                    }
                    break;
                }
                case OpCode::LoadMember: {
                    if (n.operands.load.obj > max_reg || !defined[n.operands.load.obj]) return false;
                    break;
                }
                case OpCode::StoreMember: {
                    if (n.operands.store.obj > max_reg || !defined[n.operands.store.obj]) return false;
                    if (n.operands.store.src > max_reg || !defined[n.operands.store.src]) return false;
                    break;
                }
                case OpCode::Add_I32: {
                    const auto l = n.operands.add_i32.lhs;
                    const auto r = n.operands.add_i32.rhs;
                    if (l > max_reg || !defined[l] || r > max_reg || !defined[r]) return false;
                    break;
                }
                case OpCode::Cmp_LT_I32: {
                    const auto l = n.operands.cmp_lt_i32.lhs;
                    const auto r = n.operands.cmp_lt_i32.rhs;
                    if (l > max_reg || !defined[l] || r > max_reg || !defined[r]) return false;
                    break;
                }
                case OpCode::LoadElement_I32: {
                    const auto b = n.operands.load_elem_i32.base_ptr;
                    const auto i = n.operands.load_elem_i32.index;
                    if (b > max_reg || !defined[b] || i > max_reg || !defined[i]) return false;
                    break;
                }
                default: break;
            }
        }
        switch (bb.terminator.opcode) {
            case OpCode::Branch: {
                if (bb.terminator.operands.branch_target >= ir.blocks.size()) return false;
                break;
            }
            case OpCode::BranchCond: {
                if (bb.terminator.operands.brcond.true_block >= ir.blocks.size() || bb.terminator.operands.brcond.false_block >= ir.blocks.size()) return false;
                if (bb.terminator.operands.brcond.cond > max_reg || !defined[bb.terminator.operands.brcond.cond]) return false;
                break;
            }
            case OpCode::Return: {
                if (bb.terminator.operands.ret_val != static_cast<RegisterID>(-1) && (bb.terminator.operands.ret_val > max_reg || !defined[bb.terminator.operands.ret_val])) return false;
                break;
            }
            case OpCode::ReturnVoid: break;
            case OpCode::SubroutineReturn: {
                if (bb.terminator.operands.sub_ret_val != static_cast<RegisterID>(-1) && (bb.terminator.operands.sub_ret_val > max_reg || !defined[bb.terminator.operands.sub_ret_val])) return false;
                break;
            }
            default: return false;
        }
    }
    return true;
}

}