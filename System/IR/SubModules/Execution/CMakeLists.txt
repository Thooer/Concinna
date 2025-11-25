cmake_minimum_required(VERSION 3.20)

add_library(IRSystem.Execution STATIC)
set_target_properties(IRSystem.Execution PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_sources(IRSystem.Execution PRIVATE 
  Interface/Runtime/Interpreter.ixx 
  Interface/Runtime/JIT.ixx
  Interface/Validator/Validator.ixx
  Interface/Validation/ValidateReport.ixx
  Interface/Optimizer/Pass.ixx
  Interface/Optimizer/PassManager.ixx
  Interface/Optimizer/CanonicalizeArithmetic.ixx
  Interface/Optimizer/CfgCleanupPasses.ixx
  Interface/Optimizer/CommonSubexpressionElimination.ixx
  Interface/Optimizer/ConstantFolding.ixx
  Interface/Optimizer/ConstantPropagation.ixx
  Interface/Optimizer/SimplifyBranchCondConst.ixx
  Interface/CategoryRuntime.ixx
  Interface/stub.ixx
)
target_link_libraries(IRSystem.Execution PRIVATE IRSystem.IR)