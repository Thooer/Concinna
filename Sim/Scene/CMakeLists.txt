add_module_from_dirs(Sim.Scene
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_link_libraries(Sim.Scene PRIVATE Lang Cap.Memory)
apply_common_options(Sim.Scene)
