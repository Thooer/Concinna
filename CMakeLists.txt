# NovaProject/CMakeLists.txt

cmake_minimum_required(VERSION 3.28)
project(NovaProject LANGUAGES C CXX)

# Force MSVC compiler - GCC is not supported
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "\n"
        "═══════════════════════════════════════════════════════════════════════════════════════════════════\n"
        "║                                        ❌ 构建错误 ❌                                           ║\n"
        "║                                                                                                 ║\n"
        "║  检测到 GCC 编译器，但 Nova 引擎目前仅支持 MSVC 编译器。                                             ║\n"
        "║                                                                                                 ║\n"
        "║  原因：GCC 的 C++23 模块支持尚不完整，无法正确编译 Nova 的模块化代码。                                 ║\n"
        "║                                                                                                 ║\n"
        "║  解决方案：                                                                                       ║\n"
        "║    1. 使用 Visual Studio 2022 (17.0 或更高版本)                                                    ║\n"
        "║    2. 或使用 Visual Studio Build Tools 2022                                                      ║\n"
        "║    3. 确保从 'x64 Native Tools Command Prompt for VS 2022' 运行 CMake                            ║\n"
        "║                                                                                                 ║\n"
        "║  当前检测到的编译器：${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}                        ║\n"
        "║  要求的编译器：MSVC 19.30+ (Visual Studio 2022)                                                    ║\n"
        "║                                                                                                 ║\n"
        "═══════════════════════════════════════════════════════════════════════════════════════════════════\n")
endif()

# Set C++23 standard for the entire project
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set project root
set(ROOT "${CMAKE_SOURCE_DIR}" CACHE PATH "Nova project root")

if(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# -----------------------------
# Global Nova build options and helpers
# Toggle according to project philosophy: exceptions/RTTI are disabled by default.
option(ENABLE_EXCEPTIONS "Enable C++ exceptions for Nova targets" OFF)
option(ENABLE_RTTI "Enable RTTI for Nova targets" OFF)
option(WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(RULES_STRICT "Fail build on rule violations" OFF)

# Default VS props path for disabling exceptions (VS generator)
set(NOEXCEPTION_PROPS_PATH "${CMAKE_BINARY_DIR}/Nova.NoExceptions.props" CACHE FILEPATH "VS props to disable exceptions")

# Pre-create VS props file when using MSVC and exceptions disabled
if(MSVC AND NOT ENABLE_EXCEPTIONS)
  file(WRITE "${NOEXCEPTION_PROPS_PATH}" "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n  <ItemDefinitionGroup>\n    <ClCompile>\n      <ExceptionHandling>false</ExceptionHandling>\n      <RuntimeTypeInfo>false</RuntimeTypeInfo>\n      <PreprocessorDefinitions>_HAS_EXCEPTIONS=0;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n      <AdditionalOptions>/U_CPPUNWIND %(AdditionalOptions)</AdditionalOptions>\n    </ClCompile>\n  </ItemDefinitionGroup>\n</Project>\n")
endif()

# Remove rule/CI auxiliary targets to streamline build

# Remove Ninja-specific RC/MT discovery to simplify toolchain handling

# Apply C++23, warning level, exceptions/RTTI policy to a target
function(apply_common_options target)
  target_compile_features(${target} PUBLIC cxx_std_23)
  set_target_properties(${target} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    INTERPROCEDURAL_OPTIMIZATION TRUE)

  # Ensure MSVC uses the correct language standard for modules/import
  if(MSVC)
    # Explicitly set latest C++ standard to enable 'import' on all targets
    target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:/std:c++latest>)
  endif()

  # Exceptions/RTTI defines and flags
  if(ENABLE_EXCEPTIONS)
    target_compile_definitions(${target} PUBLIC CFG_EXCEPTIONS=1)
  else()
    target_compile_definitions(${target} PUBLIC CFG_EXCEPTIONS=0 _HAS_EXCEPTIONS=0)
    if(MSVC)
      # 仅明确取消 _CPPUNWIND，真正关闭由 VS 属性表完成
      target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:/U_CPPUNWIND>)
    else()
      target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
    endif()
    if(NOT DEFINED NOEXCEPTION_PROPS_PATH)
      set(NOEXCEPTION_PROPS_PATH "${CMAKE_BINARY_DIR}/Nova.NoExceptions.props")
      file(WRITE "${NOEXCEPTION_PROPS_PATH}" "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n  <ItemDefinitionGroup>\n    <ClCompile>\n      <ExceptionHandling>false</ExceptionHandling>\n      <RuntimeTypeInfo>false</RuntimeTypeInfo>\n      <PreprocessorDefinitions>_HAS_EXCEPTIONS=0;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n      <AdditionalOptions>/U_CPPUNWIND %(AdditionalOptions)</AdditionalOptions>\n    </ClCompile>\n  </ItemDefinitionGroup>\n</Project>\n")
    endif()
    # 避免重复插入，确保 VS_USER_PROPS 只包含一次该 props
    get_property(_vsprops TARGET ${target} PROPERTY VS_USER_PROPS)
    if(NOT _vsprops)
      set_property(TARGET ${target} PROPERTY VS_USER_PROPS "${NOEXCEPTION_PROPS_PATH}")
    elseif(NOT ";${_vsprops};" MATCHES ";${NOEXCEPTION_PROPS_PATH};")
      set_property(TARGET ${target} PROPERTY VS_USER_PROPS "${_vsprops};${NOEXCEPTION_PROPS_PATH}")
    endif()
  endif()

  if(ENABLE_RTTI)
    target_compile_definitions(${target} PUBLIC CFG_RTTI=1)
  else()
    target_compile_definitions(${target} PUBLIC CFG_RTTI=0)
    if(MSVC)
      target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:/GR->)
    else()
      target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
    endif()
  endif()

  # Warnings
  if(MSVC)
    target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:/permissive- /W4>)
    # Suppress Windows SDK warnings
    target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:/D_CRT_SECURE_NO_WARNINGS>)
    if(WARNINGS_AS_ERRORS)
      target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:/WX>)
    endif()
  else()
    target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic>)
    if(WARNINGS_AS_ERRORS)
      target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-Werror>)
    endif()
  endif()
endfunction()
include(Script/CMake/NovaBuild.cmake)


# Ensure MSVC default flags from the VS generator don't re-enable exceptions/RTTI
if(MSVC)
  # Strip default exception flags when exceptions are disabled
  if(NOT ENABLE_EXCEPTIONS)
    foreach(var IN ITEMS CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
      if(DEFINED ${var})
        string(REPLACE "/EHsc" "" ${var} "${${var}}")
        string(REPLACE "/EHa"  "" ${var} "${${var}}")
        # Persist change for VS generator by updating cache
        set(${var} "${${var}}" CACHE STRING "" FORCE)
      endif()
    endforeach()
  endif()

  # Strip default RTTI flag when RTTI is disabled
  if(NOT ENABLE_RTTI)
    foreach(var IN ITEMS CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
      if(DEFINED ${var})
        string(REPLACE "/GR" "" ${var} "${${var}}")
        set(${var} "${${var}}" CACHE STRING "" FORCE)
      endif()
    endforeach()
  endif()
endif()

# Add Primitive modules
add_subdirectory(Primitive/Element)
add_subdirectory(Primitive/Semantics)
add_subdirectory(Primitive/Meta)
add_subdirectory(Primitive/Flow)
add_subdirectory(Primitive/Paradigm)
add_subdirectory(Primitive/Sync)
add_subdirectory(Primitive/Text)
add_subdirectory(Primitive/Platform)
add_subdirectory(Primitive/IO)
add_subdirectory(Primitive/File)
add_subdirectory(Primitive/Input)
add_subdirectory(Primitive/Audio)
add_subdirectory(Primitive/Debug)
add_subdirectory(Primitive/Clipboard)
add_subdirectory(Primitive/System)
add_subdirectory(Primitive/Time)
add_subdirectory(Primitive/Ownership)
add_subdirectory(Primitive/Threading)
add_subdirectory(Primitive/Socket)
add_subdirectory(Primitive/Window)
option(BUILD_PRIMITIVE_WSI "Build Primitive.WSI module" OFF)
if(BUILD_PRIMITIVE_WSI)
  add_subdirectory(Primitive/WSI)
endif()
option(BUILD_PRIMITIVE_DYNAMICLIB "Build Primitive.DynamicLibrary module" OFF)
if(BUILD_PRIMITIVE_DYNAMICLIB)
  add_subdirectory(Primitive/DynamicLibrary)
endif()

# Remove testing stubs
