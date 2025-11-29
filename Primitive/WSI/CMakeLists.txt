add_module_from_dirs(WSI
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/API
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Impl
)
target_link_libraries(WSI PRIVATE Element Flow Semantics Paradigm Text Window)
apply_common_options(WSI)

# 可选 Vulkan 支持
option(WSI_ENABLE_VULKAN "Enable Vulkan WSI support" OFF)
if(WSI_ENABLE_VULKAN AND WIN32)
  find_package(Vulkan QUIET)
  if(Vulkan_FOUND)
    target_compile_definitions(WSI PRIVATE HAS_VULKAN)
    target_link_libraries(WSI PRIVATE Vulkan::Vulkan)
  endif()
endif()

Sample(WSI.TriangleDemo ${CMAKE_CURRENT_SOURCE_DIR}/Samples/TriangleDemo OPTION BUILD_WSI_TRIANGLE LINK "WSI;Window;Time;Profiling")
