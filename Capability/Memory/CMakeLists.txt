message(STATUS "Memory CMakeLists starting (auto-collect by folders)")

add_module_from_dirs(Memory
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Backends
)
# Link direct prerequisites to ensure MSVC module scan can import
target_link_libraries(Memory PRIVATE Lang Flow Ownership Sync)
target_compile_definitions(Memory PRIVATE NOVA_FOUNDATION_BUILD)

option(NOVA_BUILD_FOUNDATION_MEMORY_SAMPLES "Build Memory samples" OFF)
if(NOVA_BUILD_FOUNDATION_MEMORY_SAMPLES)
  add_executable(Memory.SystemAllocatorDemo
    ${CMAKE_CURRENT_SOURCE_DIR}/Samples/SystemAllocatorDemo/SystemAllocatorDemo.cppm)
  target_link_libraries(Memory.SystemAllocatorDemo PRIVATE Lang Memory Platform)
  apply_common_options(Memory.SystemAllocatorDemo)

  add_executable(Memory.FrameAllocatorUsage
    ${CMAKE_CURRENT_SOURCE_DIR}/Samples/FrameAllocatorUsage/FrameAllocatorUsage.cppm)
  target_link_libraries(Memory.FrameAllocatorUsage PRIVATE Lang Memory Platform)
  apply_common_options(Memory.FrameAllocatorUsage)

  add_executable(Memory.StackAllocatorUsage
    ${CMAKE_CURRENT_SOURCE_DIR}/Samples/StackAllocatorUsage/StackAllocatorUsage.cppm)
  target_link_libraries(Memory.StackAllocatorUsage PRIVATE Lang Memory Platform)
  apply_common_options(Memory.StackAllocatorUsage)

  add_executable(Memory.PoolAllocatorUsage
    ${CMAKE_CURRENT_SOURCE_DIR}/Samples/PoolAllocatorUsage/PoolAllocatorUsage.cppm)
  target_link_libraries(Memory.PoolAllocatorUsage PRIVATE Lang Memory Platform)
  apply_common_options(Memory.PoolAllocatorUsage)

  add_executable(Memory.SmallObjectAllocatorUsage
    ${CMAKE_CURRENT_SOURCE_DIR}/Samples/SmallObjectAllocatorUsage/SmallObjectAllocatorUsage.cppm)
  target_link_libraries(Memory.SmallObjectAllocatorUsage PRIVATE Lang Memory Platform)
  apply_common_options(Memory.SmallObjectAllocatorUsage)

  add_executable(Memory.DebugProxyTest
    ${CMAKE_CURRENT_SOURCE_DIR}/Samples/DebugProxyTest/DebugProxyTest.cppm)
  target_link_libraries(Memory.DebugProxyTest PRIVATE Lang Memory Platform)
  apply_common_options(Memory.DebugProxyTest)

  add_executable(Memory.ConcurrentPoolAllocatorTest
    ${CMAKE_CURRENT_SOURCE_DIR}/Samples/ConcurrentPoolAllocatorTest/ConcurrentPoolAllocatorTest.cppm)
  target_link_libraries(Memory.ConcurrentPoolAllocatorTest PRIVATE Lang Memory Platform)
  apply_common_options(Memory.ConcurrentPoolAllocatorTest)

  add_executable(Memory.AllocatorFuzzTest
    ${CMAKE_CURRENT_SOURCE_DIR}/Samples/AllocatorFuzzTest/AllocatorFuzzTest.cppm)
  target_link_libraries(Memory.AllocatorFuzzTest PRIVATE Lang Memory Platform)
  apply_common_options(Memory.AllocatorFuzzTest)

  add_executable(Memory.HighIntensityStress
    ${CMAKE_CURRENT_SOURCE_DIR}/Samples/HighIntensityStress/HighIntensityStress.cppm)
  target_link_libraries(Memory.HighIntensityStress PRIVATE Lang Memory Platform)
  apply_common_options(Memory.HighIntensityStress)
endif()
