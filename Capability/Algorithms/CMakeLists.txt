message(STATUS "Algorithms CMakeLists starting")

add_module_from_dirs(Algorithms
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Interface
  BACKEND_DIRS
)
target_link_libraries(Algorithms PRIVATE Language Math)
apply_common_options(Algorithms)

option(NOVA_BUILD_ALGORITHMS_TESTS "Build Algorithms tests" ON)
if(NOVA_BUILD_ALGORITHMS_TESTS)
  add_executable(AlgorithmsTest ${CMAKE_CURRENT_SOURCE_DIR}/Sample/AlgorithmsTest.cppm ${CMAKE_CURRENT_SOURCE_DIR}/Sample/Main.cpp)
  target_sources(AlgorithmsTest
    PUBLIC
      FILE_SET CXX_MODULES TYPE CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Sample
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/Sample/AlgorithmsTest.cppm
  )
  target_link_libraries(AlgorithmsTest PRIVATE Language Algorithms)
  apply_common_options(AlgorithmsTest)
endif()
