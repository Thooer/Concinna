message(STATUS "Random CMakeLists starting")

add_module_from_dirs(Random
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_link_libraries(Random PRIVATE Language)

option(NOVA_BUILD_FOUNDATION_RANDOM_SAMPLES "Build Random samples" OFF)
if(NOVA_BUILD_FOUNDATION_RANDOM_SAMPLES)
  add_executable(Random.Smoke ${CMAKE_CURRENT_SOURCE_DIR}/Samples/Smoke/Smoke.cppm)
  target_link_libraries(Random.Smoke PRIVATE Language Random Platform)
  apply_common_options(Random.Smoke)
endif()

