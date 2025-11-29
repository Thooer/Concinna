message(STATUS "Sys.Job.Test CMakeLists starting")
nova_add_module_from_dirs(Sys.Job.Test
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_link_libraries(Sys.Job.Test PRIVATE Language Memory Sync Concurrency Sys)
apply_common_options(Sys.Job.Test)
