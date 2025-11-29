add_module_from_dirs(Log
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_link_libraries(Log PUBLIC Lang Platform Concurrency Cap.Debug Threading SystemMemory)
add_dependencies(Log Threading SystemMemory Cap.Debug Concurrency)
apply_common_options(Log)
