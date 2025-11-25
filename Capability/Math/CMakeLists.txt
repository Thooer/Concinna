add_module_from_dirs(Math
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Backends
)
target_link_libraries(Math PRIVATE Language SIMD System)
apply_common_options(Math)

add_executable(PacketMathTest ${CMAKE_CURRENT_SOURCE_DIR}/Samples/PacketMathTest.cppm)
target_sources(PacketMathTest
  PUBLIC
    FILE_SET CXX_MODULES TYPE CXX_MODULES
      BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Samples
      FILES ${CMAKE_CURRENT_SOURCE_DIR}/Samples/PacketMathTest.cppm
)
target_link_libraries(PacketMathTest PRIVATE Language SIMD System Math Test)
apply_common_options(PacketMathTest)

add_executable(ExtendedMathTest ${CMAKE_CURRENT_SOURCE_DIR}/Samples/ExtendedMathTest.cppm)
target_sources(ExtendedMathTest
  PUBLIC
    FILE_SET CXX_MODULES TYPE CXX_MODULES
      BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Samples
      FILES ${CMAKE_CURRENT_SOURCE_DIR}/Samples/ExtendedMathTest.cppm
)
target_link_libraries(ExtendedMathTest PRIVATE Language SIMD System Math Test)
apply_common_options(ExtendedMathTest)