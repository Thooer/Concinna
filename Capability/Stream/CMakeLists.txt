add_module_from_dirs(Stream
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/API
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Impl
)
target_link_libraries(Stream PUBLIC Lang IO)
apply_common_options(Stream)
