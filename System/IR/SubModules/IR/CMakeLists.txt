cmake_minimum_required(VERSION 3.20)

add_library(IRSystem.IR STATIC)
set_target_properties(IRSystem.IR PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_sources(IRSystem.IR PRIVATE 
  Interface/Core/Types.ixx
  Interface/Core/TypeMeta.ixx
  Interface/Core/ObjectDescriptor.ixx
  Interface/Core/Diagnostics.ixx
  Interface/Core/Object.ixx
  Interface/Core/ObjectRegistry.ixx
  Interface/Core/Utility.ixx
  Interface/CPO.ixx
  Interface/Capabilities.ixx
  Interface/IR/OpCode.ixx
  Interface/IR/MorphismIR.ixx
  Interface/IR/MorphismIRMetadata.ixx
  Interface/IR/MorphismIRValidator.ixx
  Interface/IR/FlowBuilder.ixx
  Interface/IR/FlowPipeline.ixx
  Interface/IR/IRDumper.ixx
  Interface/IR/IRMetaExplorer.ixx
  Interface/IRTypes.ixx
  Interface/Term/Term.ixx
)