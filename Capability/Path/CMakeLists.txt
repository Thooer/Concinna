add_module_from_dirs(Path
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/API
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Impl
)
target_link_libraries(Path PUBLIC Lang)
apply_common_options(Path)
