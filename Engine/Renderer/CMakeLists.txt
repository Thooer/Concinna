message(STATUS "Engine.Renderer CMakeLists starting")

nova_add_module_from_dirs(Eng.Renderer
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Default
  EXCLUDE_FILES
)
target_link_libraries(Eng.Renderer PRIVATE Lang Window WSI Time SysRHI Math)
if(TARGET Eng.Scene)
  target_link_libraries(Eng.Renderer PRIVATE Eng.Scene)
  add_dependencies(Eng.Renderer Eng.Scene)
endif()


option(NOVA_BUILD_ENGINE_RENDERER_SAMPLES "Build Engine.Renderer samples" OFF)
if(NOVA_BUILD_ENGINE_RENDERER_SAMPLES)
  set(_renderer_link "Language;Foundation.Math;Foundation.Time;Foundation.IO;Platform;SysRHI")
  if(TARGET Eng.Scene)
    set(_renderer_link "${_renderer_link};Eng.Scene")
  endif()
  nova_add_module_samples(Eng.Renderer ${CMAKE_CURRENT_SOURCE_DIR}/Samples LINK "${_renderer_link}")
endif()

Sample(Eng.Renderer.RHISimpleTriangle ${CMAKE_CURRENT_SOURCE_DIR}/Samples/RHISimpleTriangle OPTION BUILD_ENG_RENDERER_RHISIMPLETRIANGLE LINK "Lang;SysRHI;Eng.Renderer")
