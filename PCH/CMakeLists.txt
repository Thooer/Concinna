add_module_from_dirs(Lang
  INTERFACE_DIR ${CMAKE_SOURCE_DIR}/Language/API
)
target_link_libraries(Lang PUBLIC Element Semantics Flow Paradigm Text Reflection Meta)
apply_common_options(Lang)

add_library(Platform INTERFACE)
target_link_libraries(Platform INTERFACE System Time Threading Ownership Sync IO Socket Window Input Audio Clipboard Debug)

add_library(Platform.Api INTERFACE)
target_link_libraries(Platform.Api INTERFACE Platform)
