add_module_from_dirs(ConcurrentContainers
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/API
)
target_link_libraries(ConcurrentContainers PUBLIC Lang Memory Platform)
