add_module_from_dirs(SysJob
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/API
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Impl
)
target_link_libraries(SysJob PUBLIC Lang Memory Platform System Threading Sync Ownership ConcurrentContainers Tools.Tracy Concurrency Time SystemMemory)
add_dependencies(SysJob Threading)

 
