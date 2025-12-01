message(STATUS "Engine.Physics CMakeLists starting")

nova_add_module_from_dirs(Engine.Physics
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Backends
)
target_link_libraries(Engine.Physics PRIVATE Language Cap.Containers Cap.Memory)

option(NOVA_BUILD_ENGINE_PHYSICS_SAMPLES "Build Engine.Physics samples" OFF)
if(NOVA_BUILD_ENGINE_PHYSICS_SAMPLES)
  nova_add_module_samples(Engine.Physics ${CMAKE_CURRENT_SOURCE_DIR}/Samples LINK "Language;Cap.Containers;Cap.Memory;Foundation.Time;Engine.Physics")
endif()