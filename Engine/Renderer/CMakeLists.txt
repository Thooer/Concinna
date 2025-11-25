message(STATUS "Engine.Renderer CMakeLists starting")

find_package(Vulkan QUIET)
set(NOVA_RENDERER_BACKENDS ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Default)
if(Vulkan_FOUND)
  list(APPEND NOVA_RENDERER_BACKENDS ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Vulkan)
  message(STATUS "Renderer: Vulkan FOUND, enabling Vulkan backend")
else()
  message(STATUS "Renderer: Vulkan NOT found, using CPU/default backend only")
endif()

nova_add_module_from_dirs(Engine.Renderer
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${NOVA_RENDERER_BACKENDS}
)
target_link_libraries(Engine.Renderer PRIVATE Language Foundation.Math Foundation.Memory Foundation.Containers Foundation.Time Platform)
target_link_libraries(Engine.Renderer PRIVATE gdi32)
if(TARGET Engine.Scene)
  target_link_libraries(Engine.Renderer PRIVATE Engine.Scene)
  add_dependencies(Engine.Renderer Engine.Scene)
endif()
if(Vulkan_FOUND)
  target_link_libraries(Engine.Renderer PRIVATE Vulkan::Vulkan)
endif()

option(NOVA_BUILD_ENGINE_RENDERER_SAMPLES "Build Engine.Renderer samples" OFF)
if(NOVA_BUILD_ENGINE_RENDERER_SAMPLES)
  set(_renderer_link "Language;Foundation.Math;Foundation.Time;Foundation.IO;Platform;Engine.Renderer")
  if(TARGET Engine.Scene)
    set(_renderer_link "${_renderer_link};Engine.Scene")
  endif()
  nova_add_module_samples(Engine.Renderer ${CMAKE_CURRENT_SOURCE_DIR}/Samples LINK "${_renderer_link}")
endif()