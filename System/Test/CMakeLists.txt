add_module_from_dirs(Test
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_link_libraries(Test PRIVATE Language Memory)
apply_common_options(Test)
