add_module_from_dirs(Concurrency
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_link_libraries(Concurrency PUBLIC Lang Memory Platform Tools.Tracy Threading Sync Ownership ConcurrentContainers)
add_dependencies(Concurrency Threading)
apply_common_options(Concurrency)
