message(STATUS "Algorithms CMakeLists starting")

add_module_from_dirs(Algorithms
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS
)
target_link_libraries(Algorithms PRIVATE Language Containers Memory Platform)
apply_common_options(Algorithms)