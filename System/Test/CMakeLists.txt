message(STATUS "System.Test CMakeLists starting")

API(Sys.Test ${CMAKE_CURRENT_SOURCE_DIR}/Interface LINK Lang;Window;Time;SysRHI)
Impl(Sys.Test.Default ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Default LINK Lang;Window;Time;SysRHI)
Build(SysTest TYPE STATIC BACKEND Sys.Test.Default LINK Lang;Window;Time;SysRHI)
add_dependencies(SysTest SysRHI)
