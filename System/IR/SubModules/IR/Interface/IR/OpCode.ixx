export module Foundation.IRSystem:IR.IR.OpCode;
import Foundation.IRSystem:IR.Core.Types;

export namespace Foundation::IRSystem {
    enum class OpCode : UInt16 {
        Const,
        Arg,
        Move,
        LoadMember,
        StoreMember,
        Call,
        Add_I32,
        Cmp_LT_I32,
        LoadElement_I32,
        Return,
        ReturnVoid,
        SubroutineReturn,
        Branch,
        BranchCond,
        CreateActor,
        Send,
        Receive
    };
}