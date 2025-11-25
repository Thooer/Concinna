export module Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.IR.OpCode;
import <variant>;
import <vector>;

export namespace Foundation::IRSystem {

using Value = std::variant<bool, int, double, void*>;
using RegisterID = UInt32;
using ConstantID = UInt32;
using FieldIndex = UInt32;
using BlockID = UInt32;
using FunctionID = UInt64;

struct Node {
    OpCode      opcode;
    RegisterID  result_reg;
    union {
        ConstantID  const_id;
        UInt32      arg_index;
        struct { RegisterID src; } move;
        struct { RegisterID obj; FieldIndex field; } load;
        struct { RegisterID obj; FieldIndex field; RegisterID src; } store;
        struct { RegisterID lhs; RegisterID rhs; } add_i32;
        struct { RegisterID lhs; RegisterID rhs; } cmp_lt_i32;
        struct { RegisterID base_ptr; RegisterID index; } load_elem_i32;
        struct { FunctionID target_id; UInt32 args_offset; UInt32 args_count; } call;
        struct { FunctionID init_id; } create_actor;
        struct { RegisterID actor; RegisterID msg; } send;
        struct { RegisterID inbox; } receive;
        RegisterID  ret_val;
        RegisterID  sub_ret_val;
        BlockID     branch_target;
        struct { RegisterID cond; BlockID true_block; BlockID false_block; } brcond;
    } operands;
};

struct BasicBlock {
    std::vector<Node> nodes;
    Node terminator{ OpCode::ReturnVoid, static_cast<RegisterID>(-1), { .branch_target = 0 } };
    bool hasExplicitTerminator = false;
};

struct MorphismIR {
    UInt64 morphismId{};
    UInt32 version = 1;
    std::vector<Value>      constant_pool;
    std::vector<BasicBlock> blocks;
    BlockID                 entry_block = 0;
    std::vector<RegisterID> call_arg_regs;
    USize                   expected_args{};
};

}
