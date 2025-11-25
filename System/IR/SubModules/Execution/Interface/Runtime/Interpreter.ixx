export module Foundation.IRSystem:Execution.Runtime.Interpreter;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.Core.Diagnostics;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.Core.ObjectDescriptor;
import Foundation.IRSystem:IR.IR.OpCode;
import <vector>;
import <optional>;
import <stdexcept>;
import <span>;

export namespace Foundation::IRSystem {

struct StackFrame { const MorphismIR* ir; const ObjectDescriptor* obj_desc; BlockID block; USize node_index; std::vector<Value> registers; std::vector<Value> args; BlockID return_block{0}; USize return_node_index{0}; RegisterID caller_result_reg{static_cast<RegisterID>(-1)}; };

inline std::vector<TraceFrame> capture_callstack(const std::vector<StackFrame>& stack) { std::vector<TraceFrame> cs; cs.reserve(stack.size()); for (const auto& fr : stack) { cs.push_back(TraceFrame{ 0, fr.block, fr.node_index }); } return cs; }
inline RegisterID compute_max_reg(const MorphismIR& ir) { RegisterID max_reg = 0; for (const auto& blk : ir.blocks) { for (const auto& node : blk.nodes) { if (node.result_reg != static_cast<RegisterID>(-1)) { max_reg = std::max(max_reg, node.result_reg); } } } return max_reg; }

export inline RunResult interpret_fast(const MorphismIR& ir, const ObjectDescriptor& obj_desc, std::span<const Value> args) {
    if (ir.expected_args != args.size()) { ExecError err{ ErrorKind::ArgumentCountMismatch, "Argument count mismatch", 0, ir.entry_block, 0, {} }; return RunResult{ .ok = false, .value = std::nullopt, .error = std::move(err) }; }
    StackFrame main{ .ir = &ir, .obj_desc = &obj_desc, .block = ir.entry_block, .node_index = 0, .registers = {}, .args = std::vector<Value>(args.begin(), args.end()), .return_block = 0, .return_node_index = 0, .caller_result_reg = static_cast<RegisterID>(-1) };
    main.registers.resize(static_cast<USize>(compute_max_reg(ir)) + 1);
    std::vector<StackFrame> call_stack; call_stack.push_back(std::move(main));
    while (!call_stack.empty()) {
        StackFrame& frame = call_stack.back(); const auto& cur_ir = *frame.ir; const auto& blk = cur_ir.blocks[frame.block];
        if (frame.node_index < blk.nodes.size()) {
            const auto& node = blk.nodes[frame.node_index];
            switch (node.opcode) {
                case OpCode::Const: { frame.registers[node.result_reg] = cur_ir.constant_pool[node.operands.const_id]; frame.node_index++; break; }
                case OpCode::Arg: { if (static_cast<USize>(node.operands.arg_index) >= frame.args.size()) { ExecError err{ ErrorKind::ArgIndexOutOfRange, "Arg index out of range", 0, frame.block, frame.node_index, capture_callstack(call_stack) }; return RunResult{ .ok = false, .value = std::nullopt, .error = std::move(err) }; } frame.registers[node.result_reg] = frame.args[node.operands.arg_index]; frame.node_index++; break; }
                case OpCode::Move: { frame.registers[node.result_reg] = frame.registers[node.operands.move.src]; frame.node_index++; break; }
                case OpCode::LoadMember: { void* obj_ptr = nullptr; if (std::holds_alternative<void*>(frame.registers[node.operands.load.obj])) { obj_ptr = std::get<void*>(frame.registers[node.operands.load.obj]); } else { ExecError err{ ErrorKind::ImplicitThisNotAllowed, "Implicit 'this' not allowed; object must be in a register", 0, frame.block, frame.node_index, capture_callstack(call_stack) }; return RunResult{ .ok = false, .value = std::nullopt, .error = std::move(err) }; } if (node.operands.load.field >= frame.obj_desc->fields.size()) { ExecError err{ ErrorKind::InvalidFieldIndex, "Field index out of range in LoadMember", 0, frame.block, frame.node_index, capture_callstack(call_stack) }; return RunResult{ .ok = false, .value = std::nullopt, .error = std::move(err) }; } const auto& field_desc = frame.obj_desc->fields[node.operands.load.field]; char* field_addr = static_cast<char*>(obj_ptr) + field_desc.offsetBytes; if (field_desc.fieldType == GetTypeID<bool>()) { frame.registers[node.result_reg] = *reinterpret_cast<bool*>(field_addr); } else if (field_desc.fieldType == GetTypeID<int>()) { frame.registers[node.result_reg] = *reinterpret_cast<int*>(field_addr); } else if (field_desc.fieldType == GetTypeID<void*>()) { frame.registers[node.result_reg] = *reinterpret_cast<void**>(field_addr); } else { ExecError err{ ErrorKind::UnsupportedTypeLoadMember, "Unsupported type in LoadMember", 0, frame.block, frame.node_index, capture_callstack(call_stack) }; return RunResult{ .ok = false, .value = std::nullopt, .error = std::move(err) }; } frame.node_index++; break; }
                case OpCode::StoreMember: { void* obj_ptr = nullptr; if (std::holds_alternative<void*>(frame.registers[node.operands.store.obj])) { obj_ptr = std::get<void*>(frame.registers[node.operands.store.obj]); } else { ExecError err{ ErrorKind::ImplicitThisNotAllowed, "Implicit 'this' not allowed; object must be in a register", 0, frame.block, frame.node_index, capture_callstack(call_stack) }; return RunResult{ .ok = false, .value = std::nullopt, .error = std::move(err) }; } const Value& src_val = frame.registers[node.operands.store.src]; if (node.operands.store.field >= frame.obj_desc->fields.size()) { ExecError err{ ErrorKind::InvalidFieldIndex, "Field index out of range in StoreMember", 0, frame.block, frame.node_index, capture_callstack(call_stack) }; return RunResult{ .ok = false, .value = std::nullopt, .error = std::move(err) }; } const auto& field_desc = frame.obj_desc->fields[node.operands.store.field]; char* field_addr = static_cast<char*>(obj_ptr) + field_desc.offsetBytes; std::visit([field_addr](auto&& arg) { using T = std::decay_t<decltype(arg)>; if constexpr (!std::is_same_v<T, void*>) { *reinterpret_cast<T*>(field_addr) = arg; } }, src_val); frame.node_index++; break; }
                case OpCode::Add_I32: { const Value& vl = frame.registers[node.operands.add_i32.lhs]; const Value& vr = frame.registers[node.operands.add_i32.rhs]; int a = std::get<int>(vl); int b = std::get<int>(vr); frame.registers[node.result_reg] = static_cast<int>(a + b); frame.node_index++; break; }
                case OpCode::Cmp_LT_I32: { const Value& vl = frame.registers[node.operands.cmp_lt_i32.lhs]; const Value& vr = frame.registers[node.operands.cmp_lt_i32.rhs]; int a = std::get<int>(vl); int b = std::get<int>(vr); frame.registers[node.result_reg] = static_cast<bool>(a < b); frame.node_index++; break; }
                case OpCode::LoadElement_I32: { const Value& vb = frame.registers[node.operands.load_elem_i32.base_ptr]; const Value& vi = frame.registers[node.operands.load_elem_i32.index]; void* base = std::get<void*>(vb); int idx = std::get<int>(vi); int* p = static_cast<int*>(base); frame.registers[node.result_reg] = p[idx]; frame.node_index++; break; }
                default: { frame.node_index++; break; }
            } continue;
        }
        const auto& term = blk.terminator;
        switch (term.opcode) {
            case OpCode::Return: { const Value ret = frame.registers[term.operands.ret_val]; return RunResult{ .ok = true, .value = ret, .error = std::nullopt }; }
            case OpCode::ReturnVoid: { return RunResult{ .ok = true, .value = std::nullopt, .error = std::nullopt }; }
            case OpCode::Branch: { frame.block = term.operands.branch_target; frame.node_index = 0; continue; }
            case OpCode::BranchCond: { const Value& vc = frame.registers[term.operands.brcond.cond]; bool cond = std::get<bool>(vc); frame.block = cond ? term.operands.brcond.true_block : term.operands.brcond.false_block; frame.node_index = 0; continue; }
            default: { ExecError err{ ErrorKind::InvalidTerminatorOpcode, "Invalid terminator opcode", 0, frame.block, frame.node_index, capture_callstack(call_stack) }; return RunResult{ .ok = false, .value = std::nullopt, .error = std::move(err) }; }
        }
    }
    return RunResult{ .ok = true, .value = std::nullopt, .error = std::nullopt };
}

}