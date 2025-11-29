message(STATUS "Engine.Resource CMakeLists starting")

nova_add_module_from_dirs(Engine.Resource
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Backends
)
target_link_libraries(Engine.Resource PRIVATE Lang Foundation.Memory Foundation.Containers Foundation.Time Engine.Runtime)

option(NOVA_BUILD_ENGINE_RESOURCE_SAMPLES "Build Engine.Resource samples" OFF)
if(NOVA_BUILD_ENGINE_RESOURCE_SAMPLES)
  nova_add_module_samples(Engine.Resource ${CMAKE_CURRENT_SOURCE_DIR}/Samples LINK "Lang;Foundation.Memory;Foundation.Containers;Foundation.Time;Engine.Resource")
endif()
