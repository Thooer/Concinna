add_module_from_dirs(Cap.Debug
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_link_libraries(Cap.Debug PRIVATE Lang Platform)
apply_common_options(Cap.Debug)
