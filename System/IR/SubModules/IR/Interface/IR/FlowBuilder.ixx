export module Foundation.IRSystem:IR.IR.FlowBuilder;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.IR.MorphismIR;

export namespace Foundation::IRSystem {

struct FlowBuilder {
    MorphismIR ir{};
    BlockID entry{0};
    RegisterID next_reg{0};
    FlowBuilder() { ir.blocks.emplace_back(); entry = static_cast<BlockID>(ir.blocks.size() - 1); ir.entry_block = entry; ir.expected_args = 1; }
    BasicBlock& block() { return ir.blocks[entry]; }
    const BasicBlock& block() const { return ir.blocks[entry]; }
    RegisterID arg(UInt32 index) { Node n{ OpCode::Arg, 0, {} }; n.operands.arg_index = index; n.result_reg = allocate_reg(); block().nodes.push_back(n); return n.result_reg; }
    RegisterID add_const(Value v) { ir.constant_pool.push_back(v); ConstantID cid = static_cast<ConstantID>(ir.constant_pool.size() - 1); Node n{ OpCode::Const, 0, {} }; n.operands.const_id = cid; n.result_reg = allocate_reg(); block().nodes.push_back(n); return n.result_reg; }
    RegisterID load_member(RegisterID obj, FieldIndex field) { Node n{ OpCode::LoadMember, 0, {} }; n.operands.load = { obj, field }; n.result_reg = allocate_reg(); block().nodes.push_back(n); return n.result_reg; }
    void store_member(RegisterID obj, FieldIndex field, RegisterID src) { Node n{ OpCode::StoreMember, 0, {} }; n.operands.store = { obj, field, src }; n.result_reg = allocate_reg(); block().nodes.push_back(n); }
    RegisterID add_i32(RegisterID lhs, RegisterID rhs) { Node n{ OpCode::Add_I32, 0, {} }; n.operands.add_i32 = { lhs, rhs }; n.result_reg = allocate_reg(); block().nodes.push_back(n); return n.result_reg; }
    RegisterID cmp_lt_i32(RegisterID lhs, RegisterID rhs) { Node n{ OpCode::Cmp_LT_I32, 0, {} }; n.operands.cmp_lt_i32 = { lhs, rhs }; n.result_reg = allocate_reg(); block().nodes.push_back(n); return n.result_reg; }
    RegisterID load_elem_i32(RegisterID base_ptr, RegisterID index) { Node n{ OpCode::LoadElement_I32, 0, {} }; n.operands.load_elem_i32 = { base_ptr, index }; n.result_reg = allocate_reg(); block().nodes.push_back(n); return n.result_reg; }
    void ret(RegisterID r) { Node term{ OpCode::Return, static_cast<RegisterID>(-1), {} }; term.operands.ret_val = r; block().terminator = term; block().hasExplicitTerminator = true; }
    void ret_void() { Node term{ OpCode::ReturnVoid, static_cast<RegisterID>(-1), {} }; block().terminator = term; block().hasExplicitTerminator = true; }
    MorphismIR build() const { return ir; }
    RegisterID allocate_reg() { return next_reg++; }
    BlockID new_block() { ir.blocks.emplace_back(); return static_cast<BlockID>(ir.blocks.size() - 1); }
    void switch_to(BlockID bid) { entry = bid; }
    void br(BlockID target) { Node term{ OpCode::Branch, static_cast<RegisterID>(-1), {} }; term.operands.branch_target = target; block().terminator = term; block().hasExplicitTerminator = true; }
    void br_if(RegisterID cond, BlockID t, BlockID f) { Node term{ OpCode::BranchCond, static_cast<RegisterID>(-1), {} }; term.operands.brcond.cond = cond; term.operands.brcond.true_block = t; term.operands.brcond.false_block = f; block().terminator = term; block().hasExplicitTerminator = true; }
    RegisterID move(RegisterID src) { Node n{ OpCode::Move, 0, {} }; n.operands.move.src = src; n.result_reg = allocate_reg(); block().nodes.push_back(n); return n.result_reg; }
    RegisterID call(FunctionID target, std::initializer_list<RegisterID> args) { UInt32 offset = static_cast<UInt32>(ir.call_arg_regs.size()); for (RegisterID r : args) ir.call_arg_regs.push_back(r); Node n{ OpCode::Call, 0, {} }; n.operands.call.target_id = target; n.operands.call.args_offset = offset; n.operands.call.args_count = static_cast<UInt32>(args.size()); n.result_reg = allocate_reg(); block().nodes.push_back(n); return n.result_reg; }
};

}