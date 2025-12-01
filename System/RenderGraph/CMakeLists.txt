message(STATUS "System.RenderGraph CMakeLists starting")

API(Sys.RenderGraph ${CMAKE_CURRENT_SOURCE_DIR}/Interface LINK Lang;Cap.Memory;Containers;Sim.Scene)
Impl(Sys.RenderGraph.Default ${CMAKE_CURRENT_SOURCE_DIR}/Backends/Default LINK Lang;Cap.Memory;Containers;Sim.Scene)
Build(SysRenderGraph TYPE STATIC BACKEND Sys.RenderGraph.Default LINK Lang;Cap.Memory;Containers;Sim.Scene)
