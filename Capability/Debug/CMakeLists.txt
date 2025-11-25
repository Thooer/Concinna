add_module_from_dirs(Debug
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_link_libraries(Debug PRIVATE Language Platform)
apply_common_options(Debug)