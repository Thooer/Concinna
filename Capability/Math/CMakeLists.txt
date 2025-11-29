add_module_from_dirs(Math
  INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/API
  BACKEND_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Impl
)
target_link_libraries(Math PUBLIC Lang SIMD System)
apply_common_options(Math)

option(NOVA_BUILD_FOUNDATION_MATH_SAMPLES "Build Math samples" ON)
if(NOVA_BUILD_FOUNDATION_MATH_SAMPLES)
  Sample(Math.Smoke ${CMAKE_CURRENT_SOURCE_DIR}/Samples/Smoke LINK "Lang;SIMD;Math" OPTION NOVA_BUILD_FOUNDATION_MATH_SAMPLES)
endif()
