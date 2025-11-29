message(STATUS "Engine.Renderer CMakeLists starting")

find_package(Vulkan QUIET)
set(NOVA_RENDERER_BACKENDS ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Default)
if(Vulkan_FOUND)
  list(APPEND NOVA_RENDERER_BACKENDS ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Vulkan)
  message(STATUS "Renderer: Vulkan FOUND, enabling Vulkan backend")
else()
  # Try vcpkg/manual fallback
  if(DEFINED ENV{VCPKG_ROOT})
    set(_vk_inc "$ENV{VCPKG_ROOT}/installed/x64-windows/include")
    set(_vk_lib "$ENV{VCPKG_ROOT}/installed/x64-windows/lib/vulkan-1.lib")
    if(EXISTS "${_vk_lib}")
      set(Vulkan_FOUND ON)
      list(APPEND NOVA_RENDERER_BACKENDS ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Vulkan)
      message(STATUS "Renderer: Vulkan fallback via vcpkg, enabling Vulkan backend")
    else()
      message(STATUS "Renderer: Vulkan NOT found, using CPU/default backend only")
    endif()
  else()
    # Manifest install path
    set(_vk_inc "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/include")
    set(_vk_lib "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/lib/vulkan-1.lib")
    if(EXISTS "${_vk_lib}")
      set(Vulkan_FOUND ON)
      list(APPEND NOVA_RENDERER_BACKENDS ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Vulkan)
      message(STATUS "Renderer: Vulkan fallback via vcpkg_installed, enabling Vulkan backend")
    else()
      message(STATUS "Renderer: Vulkan NOT found, using CPU/default backend only")
    endif()
  endif()
endif()

nova_add_module_from_dirs(Eng.Renderer
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${NOVA_RENDERER_BACKENDS}
  EXCLUDE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Interface/FrameGraph.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/Interface/Validator.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/Interface/Simple.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/Interface/CommandList.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Default/FrameGraph.cppm
    ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Default/Validator.cppm
    ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Default/CommandList.cppm
)
target_link_libraries(Eng.Renderer PRIVATE Lang Window WSI)
target_link_libraries(Eng.Renderer PRIVATE gdi32)
if(TARGET Eng.Scene)
  target_link_libraries(Eng.Renderer PRIVATE Eng.Scene)
  add_dependencies(Eng.Renderer Eng.Scene)
endif()
if(Vulkan_FOUND)
  if(TARGET Vulkan::Vulkan)
    target_link_libraries(Eng.Renderer PRIVATE Vulkan::Vulkan)
  elseif(DEFINED _vk_lib)
    target_link_libraries(Eng.Renderer PRIVATE "${_vk_lib}")
    if(DEFINED _vk_inc)
      target_include_directories(Eng.Renderer PRIVATE "${_vk_inc}")
    endif()
  endif()
endif()

option(NOVA_BUILD_ENGINE_RENDERER_SAMPLES "Build Engine.Renderer samples" OFF)
if(NOVA_BUILD_ENGINE_RENDERER_SAMPLES)
  set(_renderer_link "Language;Foundation.Math;Foundation.Time;Foundation.IO;Platform;Eng.Renderer")
  if(TARGET Eng.Scene)
    set(_renderer_link "${_renderer_link};Eng.Scene")
  endif()
  nova_add_module_samples(Eng.Renderer ${CMAKE_CURRENT_SOURCE_DIR}/Samples LINK "${_renderer_link}")
endif()

if(Vulkan_FOUND)
  Sample(Eng.Renderer.RHIVulkanSmoke ${CMAKE_CURRENT_SOURCE_DIR}/Samples/RHIVulkanSmoke OPTION BUILD_ENG_RENDERER_RHIVULKANSMOKE LINK "Language;Foundation.Math;Foundation.Time;Foundation.IO;Platform;Eng.Renderer")
  Sample(Eng.Renderer.RHIVulkanComputeSmoke ${CMAKE_CURRENT_SOURCE_DIR}/Samples/RHIVulkanComputeSmoke OPTION BUILD_ENG_RENDERER_RHIVULKANCOMPUTESMOKE LINK "Language;Foundation.Math;Foundation.Time;Foundation.IO;Platform;Eng.Renderer")
endif()
Sample(Eng.Renderer.RHISimpleTriangle ${CMAKE_CURRENT_SOURCE_DIR}/Samples/RHISimpleTriangle OPTION BUILD_ENG_RENDERER_RHISIMPLETRIANGLE LINK "Lang;Window;WSI;Time;Eng.Renderer")
