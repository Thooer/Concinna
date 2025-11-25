message(STATUS "Engine.Gameplay CMakeLists starting")

nova_add_module_from_dirs(Engine.Gameplay
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_link_libraries(Engine.Gameplay PRIVATE Language Engine.Scene)