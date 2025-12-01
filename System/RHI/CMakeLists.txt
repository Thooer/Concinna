message(STATUS "System.RHI CMakeLists starting")

find_package(Vulkan QUIET)

API(Sys.RHI ${CMAKE_CURRENT_SOURCE_DIR}/Interface LINK Lang;Window;WSI)
Impl(Sys.RHI.Default ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Default LINK Lang;Window;WSI)

set(_rhi_backend Sys.RHI.Default)
if(Vulkan_FOUND)
  Impl(Sys.RHI.Vulkan ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Vulkan LINK Lang;Window;WSI;Foundation.Profiling)
  set(_rhi_backend Sys.RHI.Vulkan)
  message(STATUS "System.RHI: Vulkan backend enabled")
  set(WSI_ENABLE_VULKAN ON CACHE BOOL "Enable Vulkan WSI support" FORCE)
  if(TARGET WSI)
    target_compile_definitions(WSI PRIVATE HAS_VULKAN)
  endif()
else()
  # Fallback: use VULKAN_SDK environment variable if available
  if(DEFINED ENV{VULKAN_SDK})
    set(VULKAN_SDK_ROOT "$ENV{VULKAN_SDK}")
    if(EXISTS "${VULKAN_SDK_ROOT}/Lib/vulkan-1.lib" AND EXISTS "${VULKAN_SDK_ROOT}/Include/vulkan/vulkan.h")
      set(Vulkan_FOUND ON)
      Impl(Sys.RHI.Vulkan ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Vulkan LINK Lang;Window;WSI;Foundation.Profiling)
      set(_rhi_backend Sys.RHI.Vulkan)
      message(STATUS "System.RHI: Vulkan backend enabled via VULKAN_SDK")
      # Propagate Vulkan SDK include/lib to WSI and defer SysRHI linkage until after target creation
      set(_VULKAN_LIB "${VULKAN_SDK_ROOT}/Lib/vulkan-1.lib")
      set(_VULKAN_INCLUDE "${VULKAN_SDK_ROOT}/Include")
      if(TARGET WSI)
        target_compile_definitions(WSI PRIVATE HAS_VULKAN)
        target_link_libraries(WSI PRIVATE "${VULKAN_SDK_ROOT}/Lib/vulkan-1.lib")
        target_include_directories(WSI PRIVATE "${VULKAN_SDK_ROOT}/Include")
      endif()
      # Also enable WSI's internal Vulkan path
      set(WSI_ENABLE_VULKAN ON CACHE BOOL "Enable Vulkan WSI support" FORCE)
    endif()
  endif()
endif()

Build(SysRHI TYPE STATIC BACKEND ${_rhi_backend} LINK Lang;Window;WSI)

# Ensure WSI module builds before SysRHI to satisfy C++ module import order
add_dependencies(SysRHI WSI)

if(Vulkan_FOUND)
  if(TARGET Vulkan::Vulkan)
    target_link_libraries(SysRHI PRIVATE Vulkan::Vulkan)
  elseif(EXISTS "$ENV{VCPKG_ROOT}/installed/x64-windows/lib/vulkan-1.lib")
    target_link_libraries(SysRHI PRIVATE "$ENV{VCPKG_ROOT}/installed/x64-windows/lib/vulkan-1.lib")
    target_include_directories(SysRHI PRIVATE "$ENV{VCPKG_ROOT}/installed/x64-windows/include")
  elseif(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/lib/vulkan-1.lib")
    target_link_libraries(SysRHI PRIVATE "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/lib/vulkan-1.lib")
    target_include_directories(SysRHI PRIVATE "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/include")
  endif()
endif()

# Fallback linkage via VULKAN_SDK env
if(DEFINED _VULKAN_LIB AND EXISTS "${_VULKAN_LIB}")
  target_link_libraries(SysRHI PRIVATE "${_VULKAN_LIB}")
  if(DEFINED _VULKAN_INCLUDE AND EXISTS "${_VULKAN_INCLUDE}/vulkan/vulkan.h")
    target_include_directories(SysRHI PRIVATE "${_VULKAN_INCLUDE}")
  endif()
endif()
