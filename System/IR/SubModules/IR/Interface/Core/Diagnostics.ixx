export module Foundation.IRSystem:IR.Core.Diagnostics;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.IR.MorphismIR;
import <string>;
import <vector>;
import <optional>;

export namespace Foundation::IRSystem {

enum class ErrorKind : UInt32 { ArgumentCountMismatch, InvalidBlockID, InvalidTerminatorOpcode, UnsupportedTypeLoadMember, InvalidFieldIndex, CallTargetNotRegistered, CallArgsOutOfRange, RegisterIndexOutOfRange, ConstantIndexOutOfRange, ArgIndexOutOfRange, IRValidationFailed, ImplicitThisNotAllowed, BackendNotAvailable, InvalidBranchCondition, Unknown };

struct TraceFrame { FunctionID func; BlockID block; USize node_index; };

struct ExecError { ErrorKind kind{ErrorKind::Unknown}; std::string message; FunctionID func{0}; BlockID block{0}; USize node_index{0}; std::vector<TraceFrame> callstack; };

struct RunResult { bool ok{false}; std::optional<Value> value{}; std::optional<ExecError> error{}; };

}
