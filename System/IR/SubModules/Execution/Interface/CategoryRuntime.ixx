export module Foundation.IRSystem:Execution.CategoryRuntime;
import Foundation.IRSystem:Execution.Runtime.Interpreter;
import Foundation.IRSystem:Execution.Runtime.JIT;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.Core.Diagnostics;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.Core.ObjectDescriptor;
import Lang:Views;

export namespace Foundation::IRSystem {

inline RunResult interpret_fast_adapter(const MorphismIR& ir, const ObjectDescriptor& od, Language::Span<const Value> args) {
    return interpret_fast(ir, od, args);
}

struct JITAdapter {
    static NativeFn compile(const MorphismIR& ir, const ObjectDescriptor& od, int opt_level = 0) { return Foundation::IRSystem::compile(ir, od, opt_level); }
    static RunResult run(const NativeFn& nf, Language::Span<const Value> args) { return Foundation::IRSystem::run(nf, args); }
};

}