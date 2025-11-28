message(STATUS "Crypto CMakeLists starting (auto-collect by folders)")

add_module_from_dirs(Crypto
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Impl
)

# Link direct prerequisites to ensure MSVC module scan can import
target_link_libraries(Crypto PRIVATE Containers Memory)
target_compile_definitions(Crypto PRIVATE NOVA_FOUNDATION_BUILD)

option(NOVA_BUILD_FOUNDATION_CRYPTO_SAMPLES "Build Crypto samples" OFF)
if(NOVA_BUILD_FOUNDATION_CRYPTO_SAMPLES)
  # Add samples here when they are created
endif()
