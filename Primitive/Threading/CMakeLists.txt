add_module_from_dirs(Threading
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/API
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Impl
)
target_link_libraries(Threading PUBLIC Lang Element Flow Semantics Paradigm Text Platform)

# 可选 Fiber 后端（默认 OS）：如需汇编版可扩展此段
set(THREADING_FIBER_BACKEND "OS" CACHE STRING "Fiber backend (ASM|OS)")
if(WIN32)
  # Assume Windows backend enabled by default when building on WIN32
  if(TRUE)
    if(THREADING_FIBER_BACKEND STREQUAL "ASM")
      enable_language(ASM_MASM)
      target_sources(Threading PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Impl/Windows/FiberContext.asm)
    else()
      target_sources(Threading PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Impl/Windows/FiberContextOS.cppm)
      target_compile_definitions(Threading PRIVATE THREADING_FIBER_BACKEND_OS)
    endif()
  endif()
endif()
set_target_properties(Threading PROPERTIES LINKER_Prim CXX)
