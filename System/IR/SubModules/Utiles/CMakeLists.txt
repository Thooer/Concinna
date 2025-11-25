cmake_minimum_required(VERSION 3.20)

add_library(IRSystem.Utiles STATIC)
set_target_properties(IRSystem.Utiles PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_sources(IRSystem.Utiles PRIVATE Interface/Validator.ixx Interface/IRDumper.ixx)
target_link_libraries(IRSystem.Utiles PRIVATE IRSystem.IR)