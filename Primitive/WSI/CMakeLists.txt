API(Primitive.WSI ${CMAKE_CURRENT_SOURCE_DIR}/API LINK Element;Flow;Semantics;Paradigm;Text;Window)
Impl(Primitive.WSI.Windows ${CMAKE_CURRENT_SOURCE_DIR}/Impl LINK Platform;Window)
Build(WSI TYPE STATIC BACKEND Primitive.WSI.Windows)

# 可选 Vulkan 支持
option(WSI_ENABLE_VULKAN "Enable Vulkan WSI support" OFF)
if(WSI_ENABLE_VULKAN AND WIN32)
  get_property(_wsi_backend GLOBAL PROPERTY "MODULE_Primitive.WSI_SELECTED_BACKEND")
  if(_wsi_backend STREQUAL "Windows")
    find_package(Vulkan QUIET)
    if(Vulkan_FOUND)
      target_compile_definitions(WSI PRIVATE HAS_VULKAN)
      target_link_libraries(WSI PRIVATE Vulkan::Vulkan)
    endif()
  endif()
endif()
set_target_properties(WSI PROPERTIES LINKER_Prim CXX)

