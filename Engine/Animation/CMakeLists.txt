message(STATUS "Engine.Animation CMakeLists starting")

nova_add_module_from_dirs(Engine.Animation
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_link_libraries(Engine.Animation PRIVATE Language Foundation.Math)