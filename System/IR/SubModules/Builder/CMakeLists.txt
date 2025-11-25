cmake_minimum_required(VERSION 3.20)

add_library(IRSystem.Builder STATIC)
set_target_properties(IRSystem.Builder PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_sources(IRSystem.Builder PRIVATE 
  Interface/CategoryBuilder.ixx
  Interface/Capabilities/Synthesis.ixx
  Interface/Frontend/TextFrontend.ixx
  Interface/LawsTerm/RewriteEngine.ixx
  Interface/Compiler/LoweringPipeline.ixx
)
target_link_libraries(IRSystem.Builder PRIVATE IRSystem.IR)