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

# Enable C++20 module dependency scanning across targets (MSVC/VS generator)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API 1)

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
option(BUILD_SYSTEM_TEST "Build System.Test module" OFF)
option(BUILD_ENGINE_AND_ABOVE "Build Engine layer and above (Dev/Editor/Hub/Web/Plg/Pjt)" OFF)
set(NOVA_BUILD_ALGORITHMS_TESTS OFF CACHE BOOL "Disable Algorithms tests" FORCE)
set(NOVA_BUILD_FOUNDATION_MATH_SAMPLES OFF CACHE BOOL "Disable Math samples" FORCE)

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
    # Enable full standard modules support including partitions on MSVC
    target_compile_options(${target} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:/experimental:module>)
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
include(Scripts/CMake/NovaBuild.cmake)


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

# Add Language modules (moved from Primitive)
add_subdirectory(Language/Meta)
add_subdirectory(Language/Reflection)
add_subdirectory(Language/Element)
add_subdirectory(Language/Semantics)
add_subdirectory(Language/Flow)
add_subdirectory(Language/Paradigm)
add_subdirectory(Language/Text)

# Aggregator targets (Lang, Platform) must be available early
add_subdirectory(PCH)

# Add Primitive modules
add_subdirectory(Primitive/Sync)
add_subdirectory(Primitive/IO)
add_subdirectory(Primitive/Input)
add_subdirectory(Primitive/Audio)
add_subdirectory(Primitive/SIMD)
add_subdirectory(Primitive/Debug)
add_subdirectory(Primitive/Clipboard)
add_subdirectory(Primitive/System)
add_subdirectory(Primitive/Time)
add_subdirectory(Primitive/Ownership)
add_subdirectory(Primitive/Threading)
add_subdirectory(Primitive/Socket)
add_subdirectory(Primitive/Window)
add_subdirectory(Primitive/HID)
add_subdirectory(Primitive/WSI)
option(BUILD_PRIMITIVE_DYNAMICLIB "Build Primitive.DynamicLibrary module" OFF)
if(BUILD_PRIMITIVE_DYNAMICLIB)
  add_subdirectory(Primitive/DynamicLibrary)
endif()

# Remove testing stubs

# Capability layer
add_subdirectory(Capability/ConcurrentContainers)
add_subdirectory(Capability/Containers)
add_subdirectory(Capability/Concurrency)
add_subdirectory(Capability/Memory)
add_subdirectory(Capability/Math)
add_subdirectory(Capability/Random)
add_subdirectory(Capability/Algorithms)
add_subdirectory(Capability/Debug)
add_subdirectory(Capability/Log)
add_subdirectory(Capability/Profiling)
add_subdirectory(Capability/Stream)
add_subdirectory(Capability/Path)
add_subdirectory(Capability/AsyncIO)

# System layer
add_subdirectory(System/Memory)
add_subdirectory(System/Job)
add_subdirectory(System/RHI)
add_subdirectory(System/RenderGraph)
option(BUILD_SYSTEM_TASK "Build System.Task module" OFF)
if(BUILD_SYSTEM_TASK)
  add_subdirectory(System/Task)
endif()
option(BUILD_SYSTEM_INPUT "Build System.Input module" OFF)
if(BUILD_SYSTEM_INPUT)
  add_subdirectory(System/Input)
endif()
if(BUILD_SYSTEM_TEST)
  add_subdirectory(System/Test)
endif()

# Third-party tools
add_subdirectory(ThirdParty/Tracy)

## Aggregator targets moved to PCH folder

# Foundation layer aggregators used by Engine targets
add_library(Foundation.Time INTERFACE)
target_link_libraries(Foundation.Time INTERFACE Time)

add_library(Foundation.Profiling INTERFACE)
target_link_libraries(Foundation.Profiling INTERFACE Tools.Tracy)

add_library(Cap.Containers INTERFACE)
target_link_libraries(Cap.Containers INTERFACE ConcurrentContainers)

add_library(Cap.Memory INTERFACE)
target_link_libraries(Cap.Memory INTERFACE Memory SystemMemory Ownership)

# Simulation layer
if(EXISTS "${CMAKE_SOURCE_DIR}/Simulaition/Scene")
  add_subdirectory(Simulation/Scene)
endif()

# Engine runtime (optional)
if(BUILD_ENGINE_AND_ABOVE)
  # Optional engine modules toggles
  option(BUILD_ENGINE_RENDERER "Build Engine.Renderer" ON)
  option(BUILD_ENGINE_SCENE "Build Engine.Scene" OFF)
  option(BUILD_ENGINE_RESOURCE "Build Engine.Resource" OFF)
  option(BUILD_EDITOR_LAUNCHER "Build Editor.Launcher" OFF)

  add_subdirectory(Engine/Runtime)
  if(BUILD_ENGINE_RENDERER AND EXISTS "${CMAKE_SOURCE_DIR}/Engine/Renderer")
    add_subdirectory(Engine/Renderer)
  endif()
  if(BUILD_ENGINE_SCENE AND EXISTS "${CMAKE_SOURCE_DIR}/Engine/Scene")
    add_subdirectory(Engine/Scene)
  endif()
  if(BUILD_ENGINE_RESOURCE AND EXISTS "${CMAKE_SOURCE_DIR}/Engine/Resource")
    add_subdirectory(Engine/Resource)
  endif()
  if(BUILD_EDITOR_LAUNCHER AND EXISTS "${CMAKE_SOURCE_DIR}/Editor/Launcher")
    add_subdirectory(Editor/Launcher)
  endif()
  if(EXISTS "${CMAKE_SOURCE_DIR}/Projects")
    add_subdirectory(Projects)
  endif()
endif()
