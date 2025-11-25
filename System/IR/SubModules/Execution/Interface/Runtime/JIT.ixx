export module Foundation.IRSystem:Execution.JIT;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.IR.MorphismIR;
import Foundation.IRSystem:IR.Core.ObjectDescriptor;
import Foundation.IRSystem:Execution.Interpreter;
import Foundation.IRSystem:IR.CPO;
import Foundation.IRSystem:IR.Core.Diagnostics;
import Language:Views;

export namespace Foundation::IRSystem {

struct NativeFn { MorphismIR ir; const ObjectDescriptor* desc{}; };

export inline NativeFn compile(const MorphismIR& ir, const ObjectDescriptor& desc, int opt_level = 0) { NativeFn fn{}; fn.ir = ir; fn.desc = &desc; return fn; }
export inline RunResult run(const NativeFn& fn, Language::Span<const Value> args) { if (fn.ir.blocks.size() == 1) return interpret_fast(fn.ir, *fn.desc, args); return interpret_fast(fn.ir, *fn.desc, args); }

}