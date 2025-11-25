set(PROFILING_BACKEND "Native" CACHE STRING "Profiling backend (Native|Tracy)")

if(PROFILING_BACKEND STREQUAL "Tracy")
  set(PROFILING_EXCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Native/Native.cppm)
else()
  set(PROFILING_EXCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Tracy/Tracy.cppm)
endif()

add_module_from_dirs(Profiling
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Backends
  EXCLUDE_FILES ${PROFILING_EXCLUDE}
)
target_link_libraries(Profiling PRIVATE Language Platform)
if(PROFILING_BACKEND STREQUAL "Tracy")
  target_link_libraries(Profiling PRIVATE Tools.Tracy)
endif()
apply_common_options(Profiling)