message(STATUS "Engine.Scene CMakeLists starting")

nova_add_module_from_dirs(Engine.Scene
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Backends
)
target_link_libraries(Engine.Scene PRIVATE Lang Foundation.Memory Foundation.Containers Foundation.Math)

option(NOVA_BUILD_ENGINE_SCENE_SAMPLES "Build Engine.Scene samples" OFF)
if(NOVA_BUILD_ENGINE_SCENE_SAMPLES)
  nova_add_module_samples(Engine.Scene ${CMAKE_CURRENT_SOURCE_DIR}/Samples LINK "Lang;Foundation.Memory;Foundation.Containers;Foundation.Math;Engine.Scene")
endif()
