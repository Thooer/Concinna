add_module_from_dirs(Log
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_link_libraries(Log PUBLIC Language Platform Concurrency Debug)
apply_common_options(Log)