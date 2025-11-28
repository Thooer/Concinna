message(STATUS "Containers CMakeLists starting")

add_module_from_dirs(Containers
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Backends
)
target_link_libraries(Containers PUBLIC Language Memory Platform SIMD)
apply_common_options(Containers)

option(NOVA_BUILD_FOUNDATION_CONTAINERS_SAMPLES "Build Containers samples" OFF)
if(NOVA_BUILD_FOUNDATION_CONTAINERS_SAMPLES)
  add_executable(Containers.VectorSmoke ${CMAKE_CURRENT_SOURCE_DIR}/Samples/VectorSmoke/VectorSmoke.cppm)
  target_link_libraries(Containers.VectorSmoke PRIVATE Language Memory Containers Platform)
  apply_common_options(Containers.VectorSmoke)

  add_executable(Containers.StringSmoke ${CMAKE_CURRENT_SOURCE_DIR}/Samples/StringSmoke/StringSmoke.cppm)
  target_link_libraries(Containers.StringSmoke PRIVATE Language Memory Containers Platform)
  apply_common_options(Containers.StringSmoke)

  add_executable(Containers.HashMapSmoke ${CMAKE_CURRENT_SOURCE_DIR}/Samples/HashMapSmoke/HashMapSmoke.cppm)
  target_link_libraries(Containers.HashMapSmoke PRIVATE Language Memory Containers Platform)
  apply_common_options(Containers.HashMapSmoke)

  add_executable(Containers.DequeSmoke ${CMAKE_CURRENT_SOURCE_DIR}/Samples/DequeSmoke/DequeSmoke.cppm)
  target_link_libraries(Containers.DequeSmoke PRIVATE Language Memory Containers Platform)
  apply_common_options(Containers.DequeSmoke)

  add_executable(Containers.VectorHighIntensityStress ${CMAKE_CURRENT_SOURCE_DIR}/Samples/VectorHighIntensityStress/VectorHighIntensityStress.cppm)
  target_link_libraries(Containers.VectorHighIntensityStress PRIVATE Language Memory Containers Platform)
  apply_common_options(Containers.VectorHighIntensityStress)

  add_executable(Containers.HashMapHighIntensityStress ${CMAKE_CURRENT_SOURCE_DIR}/Samples/HashMapHighIntensityStress/HashMapHighIntensityStress.cppm)
  target_link_libraries(Containers.HashMapHighIntensityStress PRIVATE Language Memory Containers Platform)
  apply_common_options(Containers.HashMapHighIntensityStress)

  add_executable(Containers.DequeHighIntensityStress ${CMAKE_CURRENT_SOURCE_DIR}/Samples/DequeHighIntensityStress/DequeHighIntensityStress.cppm)
  target_link_libraries(Containers.DequeHighIntensityStress PRIVATE Language Memory Containers Platform)
  apply_common_options(Containers.DequeHighIntensityStress)

  add_executable(Containers.StringHighIntensityStress ${CMAKE_CURRENT_SOURCE_DIR}/Samples/StringHighIntensityStress/StringHighIntensityStress.cppm)
  target_link_libraries(Containers.StringHighIntensityStress PRIVATE Language Memory Containers Platform)
  apply_common_options(Containers.StringHighIntensityStress)

  add_executable(Containers.VectorMicroBench ${CMAKE_CURRENT_SOURCE_DIR}/Samples/VectorMicroBench/VectorMicroBench.cppm)
  target_link_libraries(Containers.VectorMicroBench PRIVATE Language Memory Containers Platform)
  apply_common_options(Containers.VectorMicroBench)

  add_executable(Containers.StringMicroBench ${CMAKE_CURRENT_SOURCE_DIR}/Samples/StringMicroBench/StringMicroBench.cppm)
  target_link_libraries(Containers.StringMicroBench PRIVATE Language Memory Containers Platform)
  apply_common_options(Containers.StringMicroBench)

  add_executable(Containers.VectorFrameAllocatorStress ${CMAKE_CURRENT_SOURCE_DIR}/Samples/VectorFrameAllocatorStress/VectorFrameAllocatorStress.cppm)
  target_link_libraries(Containers.VectorFrameAllocatorStress PRIVATE Language Memory Containers Platform)
  apply_common_options(Containers.VectorFrameAllocatorStress)

  add_executable(Containers.StringFrameAllocatorStress ${CMAKE_CURRENT_SOURCE_DIR}/Samples/StringFrameAllocatorStress/StringFrameAllocatorStress.cppm)
  target_link_libraries(Containers.StringFrameAllocatorStress PRIVATE Language Memory Containers Platform)
  apply_common_options(Containers.StringFrameAllocatorStress)
endif()
