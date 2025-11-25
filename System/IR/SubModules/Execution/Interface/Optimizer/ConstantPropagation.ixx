export module Foundation.IRSystem:Execution.Optimizer.ConstantPropagation;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.IR.OpCode;
import <optional>;
import <vector>;

export namespace Foundation::IRSystem {

export inline bool run_inplace_constant_propagation(MorphismIR& ir) {
    bool changed = false;
    for (auto& blk : ir.blocks) {
        std::vector<std::optional<Value>> reg_const;
        auto ensure = [&](RegisterID r){ if (static_cast<size_t>(r) >= reg_const.size()) reg_const.resize(static_cast<size_t>(r) + 1); };
        for (auto& node : blk.nodes) {
            switch (node.opcode) {
                case OpCode::Const: {
                    ensure(node.result_reg);
                    reg_const[node.result_reg] = ir.constant_pool[node.operands.const_id];
                    break;
                }
                case OpCode::Arg: {
                    ensure(node.result_reg);
                    reg_const[node.result_reg] = std::nullopt;
                    break;
                }
                case OpCode::Move: {
                    ensure(node.result_reg);
                    ensure(node.operands.move.src);
                    reg_const[node.result_reg] = reg_const[node.operands.move.src];
                    break;
                }
                case OpCode::LoadMember: {
                    ensure(node.result_reg);
                    reg_const[node.result_reg] = std::nullopt;
                    break;
                }
                case OpCode::LoadElement_I32: {
                    ensure(node.result_reg);
                    reg_const[node.result_reg] = std::nullopt;
                    break;
                }
                case OpCode::Add_I32: {
                    ensure(node.operands.add_i32.lhs);
                    ensure(node.operands.add_i32.rhs);
                    ensure(node.result_reg);
                    if (reg_const[node.operands.add_i32.lhs].has_value() && reg_const[node.operands.add_i32.rhs].has_value()) {
                        const auto& vl = *reg_const[node.operands.add_i32.lhs];
                        const auto& vr = *reg_const[node.operands.add_i32.rhs];
                        if (std::holds_alternative<int>(vl) && std::holds_alternative<int>(vr)) {
                            int res = std::get<int>(vl) + std::get<int>(vr);
                            ir.constant_pool.push_back(res);
                            node.opcode = OpCode::Const;
                            node.operands.const_id = static_cast<UInt32>(ir.constant_pool.size() - 1);
                            reg_const[node.result_reg] = res;
                            changed = true;
                        } else {
                            reg_const[node.result_reg] = std::nullopt;
                        }
                    } else {
                        reg_const[node.result_reg] = std::nullopt;
                    }
                    break;
                }
                case OpCode::Cmp_LT_I32: {
                    ensure(node.operands.cmp_lt_i32.lhs);
                    ensure(node.operands.cmp_lt_i32.rhs);
                    ensure(node.result_reg);
                    if (reg_const[node.operands.cmp_lt_i32.lhs].has_value() && reg_const[node.operands.cmp_lt_i32.rhs].has_value()) {
                        const auto& vl = *reg_const[node.operands.cmp_lt_i32.lhs];
                        const auto& vr = *reg_const[node.operands.cmp_lt_i32.rhs];
                        if (std::holds_alternative<int>(vl) && std::holds_alternative<int>(vr)) {
                            bool res = std::get<int>(vl) < std::get<int>(vr);
                            ir.constant_pool.push_back(res);
                            node.opcode = OpCode::Const;
                            node.operands.const_id = static_cast<UInt32>(ir.constant_pool.size() - 1);
                            reg_const[node.result_reg] = res;
                            changed = true;
                        } else {
                            reg_const[node.result_reg] = std::nullopt;
                        }
                    } else {
                        reg_const[node.result_reg] = std::nullopt;
                    }
                    break;
                }
                default: {
                    if (node.result_reg != static_cast<RegisterID>(-1)) {
                        ensure(node.result_reg);
                        reg_const[node.result_reg] = std::nullopt;
                    }
                    break;
                }
            }
        }
    }
    return changed;
}

}