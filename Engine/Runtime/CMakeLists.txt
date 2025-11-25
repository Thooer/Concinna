message(STATUS "Engine.Runtime CMakeLists starting")

nova_add_module_from_dirs(Engine.Runtime
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Backends
)
target_link_libraries(Engine.Runtime PRIVATE Language Platform.Api Foundation.Time Foundation.Profiling Foundation.Containers Foundation.Memory)
add_dependencies(Engine.Runtime Language Foundation.Time Foundation.Profiling Foundation.Containers Foundation.Memory Platform.Api)

option(NOVA_BUILD_ENGINE_RUNTIME_SAMPLES "Build Engine.Runtime samples" OFF)
if(NOVA_BUILD_ENGINE_RUNTIME_SAMPLES)
  nova_add_module_samples(Engine.Runtime ${CMAKE_CURRENT_SOURCE_DIR}/Samples LINK "Language;Foundation.Time;Foundation.Profiling;Foundation.Containers;Foundation.Memory;Engine.Runtime")
  set(_ls_mod ${CMAKE_CURRENT_SOURCE_DIR}/Samples/LoopSmoke.cppm)
  set(_ls_main ${CMAKE_CURRENT_SOURCE_DIR}/Samples/LoopSmoke/main.cpp)
  if(EXISTS "${_ls_mod}" AND EXISTS "${_ls_main}")
    add_executable(Engine.Runtime.LoopSmoke)
    target_sources(Engine.Runtime.LoopSmoke
      PRIVATE
        FILE_SET CXX_MODULES TYPE CXX_MODULES
          BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Samples
          FILES ${_ls_mod}
    )
    target_sources(Engine.Runtime.LoopSmoke PRIVATE "${_ls_main}")
    target_link_libraries(Engine.Runtime.LoopSmoke PRIVATE Language Platform.Api Foundation.Time Foundation.Profiling Foundation.Containers Foundation.Memory Engine.Runtime)
    nova_apply_common_options(Engine.Runtime.LoopSmoke)
  endif()
  set(_cs_mod ${CMAKE_CURRENT_SOURCE_DIR}/Samples/ChainSmoke.cppm)
  set(_cs_main ${CMAKE_CURRENT_SOURCE_DIR}/Samples/ChainSmoke/main.cpp)
  if(EXISTS "${_cs_mod}" AND EXISTS "${_cs_main}")
    add_executable(Engine.Runtime.ChainSmoke)
    target_sources(Engine.Runtime.ChainSmoke
      PRIVATE
        FILE_SET CXX_MODULES TYPE CXX_MODULES
          BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Samples
          FILES ${_cs_mod}
    )
    target_sources(Engine.Runtime.ChainSmoke PRIVATE "${_cs_main}")
    target_link_libraries(Engine.Runtime.ChainSmoke PRIVATE Language Platform.Api Foundation.Time Foundation.Profiling Foundation.Containers Foundation.Memory Engine.Runtime)
    nova_apply_common_options(Engine.Runtime.ChainSmoke)
  endif()
  # Integration sample using Scene/Resource/Renderer modules
  set(_is_mod ${CMAKE_CURRENT_SOURCE_DIR}/Samples/IntegrationSmoke.cppm)
  set(_is_main ${CMAKE_CURRENT_SOURCE_DIR}/Samples/IntegrationSmoke/main.cpp)
  if(EXISTS "${_is_mod}" AND EXISTS "${_is_main}" AND TARGET Engine.Resource)
    add_executable(Engine.Runtime.IntegrationSmoke)
    target_sources(Engine.Runtime.IntegrationSmoke
      PRIVATE
        FILE_SET CXX_MODULES TYPE CXX_MODULES
          BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Samples
          FILES ${_is_mod}
    )
    target_sources(Engine.Runtime.IntegrationSmoke PRIVATE "${_is_main}")
    target_link_libraries(Engine.Runtime.IntegrationSmoke PRIVATE Language Platform.Api Foundation.Time Foundation.Profiling Foundation.Containers Foundation.Memory Foundation.Math Engine.Scene Engine.Resource Engine.Renderer Engine.Runtime)
    nova_apply_common_options(Engine.Runtime.IntegrationSmoke)
  endif()
  set(_ml_mod ${CMAKE_CURRENT_SOURCE_DIR}/Samples/MainlineARuntime.cppm)
  set(_ml_main ${CMAKE_CURRENT_SOURCE_DIR}/Samples/MainlineARuntime/main.cpp)
  if(EXISTS "${_ml_mod}" AND EXISTS "${_ml_main}")
    add_executable(Engine.Runtime.MainlineARuntime)
    target_sources(Engine.Runtime.MainlineARuntime
      PRIVATE
        FILE_SET CXX_MODULES TYPE CXX_MODULES
          BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Samples
          FILES ${_ml_mod}
    )
    target_sources(Engine.Runtime.MainlineARuntime PRIVATE "${_ml_main}")
    target_link_libraries(Engine.Runtime.MainlineARuntime PRIVATE Language Platform.Api Foundation.Time Foundation.Profiling Foundation.Containers Foundation.Memory Engine.Runtime Engine.Messaging)
    nova_apply_common_options(Engine.Runtime.MainlineARuntime)
  endif()
endif()

# Embedded submodules inside Engine
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Scene")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Scene)
endif()
option(NOVA_BUILD_ENGINE_RESOURCE "Build Engine.Resource submodule" OFF)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Resource")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Resource)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Renderer")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Renderer)
endif()
option(NOVA_BUILD_ENGINE_ANIMATION "Build Engine.Animation submodule" OFF)
if(NOVA_BUILD_ENGINE_ANIMATION AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Animation")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Animation)
endif()
option(NOVA_BUILD_ENGINE_PHYSICS "Build Engine.Physics submodule" OFF)
if(NOVA_BUILD_ENGINE_PHYSICS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Physics")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Physics)
endif()
option(NOVA_BUILD_ENGINE_GAMEPLAY "Build Engine.Gameplay submodule" OFF)
if(NOVA_BUILD_ENGINE_GAMEPLAY AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Gameplay")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Gameplay)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Messaging")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SubModules/Messaging)
endif()
