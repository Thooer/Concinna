add_module_from_dirs(Tools.Tracy
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Core
)
target_include_directories(Tools.Tracy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public)
target_compile_definitions(Tools.Tracy PUBLIC TRACY_ENABLE)
apply_common_options(Tools.Tracy)