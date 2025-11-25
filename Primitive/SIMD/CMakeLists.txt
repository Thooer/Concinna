add_module_from_dirs(SIMD
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_link_libraries(SIMD PRIVATE Language System)
apply_common_options(SIMD)