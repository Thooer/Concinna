cmake_minimum_required(VERSION 3.20)
project(CategorySplit LANGUAGES CXX)

# Nova/SubModules/Language/CMakeLists.txt
message(STATUS "Language CMakeLists.txt starting")

# Provide a local stub for nova_apply_common_options for standalone builds
function(nova_apply_common_options target)
  # Prefer C++23, require standard without extensions
  set_target_properties(${target} PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
  target_compile_features(${target} PUBLIC cxx_std_23)
  if (MSVC)
    target_compile_options(${target} PRIVATE /permissive- /W4 /Zc:__cplusplus)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endfunction()

# 语言模块已整合 Core 接口至 Language/Interface，移除旧的 SubModules/Core 引用

# -----------------------------
# Create Language as a non-module static library (header-only contents)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Language/Language.cpp")
  add_library(Language STATIC)
  nova_apply_common_options(Language)
  target_include_directories(Language PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Language)
  target_sources(Language PRIVATE Language/Language.cpp)
else()
  message(STATUS "Skipping Language static library: Language/Language.cpp not found")
endif()


# Helper functions to add samples only if sources exist
function(nova_add_language_sample target sample_dir)
  set(sample_src "${CMAKE_CURRENT_SOURCE_DIR}/${sample_dir}/Main.cpp")
  if(EXISTS "${sample_src}")
    add_executable(${target})
    target_link_libraries(${target} PRIVATE Language)
    target_sources(${target} PRIVATE "${sample_src}")
    nova_apply_common_options(${target})
  else()
    message(STATUS "Skipping sample ${target}: ${sample_src} not found")
  endif()
endfunction()

# API samples helper: link against new Language(API) target
function(nova_add_language_api_sample target sample_dir)
  set(sample_src "${CMAKE_CURRENT_SOURCE_DIR}/${sample_dir}/Main.cpp")
  if(EXISTS "${sample_src}")
    message(STATUS "Skipping API sample ${target}: Language(API) target not available")
  else()
    message(STATUS "Skipping API sample ${target}: ${sample_src} not found")
  endif()
endfunction()

## 已移除带异常的样例生成器以符合引擎禁用异常的约束

option(NOVA_BUILD_LANGUAGE_SAMPLES "Build Language module samples" OFF)
if(NOVA_BUILD_LANGUAGE_SAMPLES)
  # Samples guarded by existence checks
  nova_add_language_sample(Language.BindBackSample Samples/BindBackSample)
  nova_add_language_sample(Language.CallableTraitsSample Samples/CallableTraitsSample)
  nova_add_language_sample(Language.FenceSample Samples/FenceSample)
  nova_add_language_sample(Language.EnumFlagsSample Samples/EnumFlagsSample)
  # 移除懒加载样例，避免误导为“假 AOT”路径
  # nova_add_language_sample(Language.LazySample Samples/LazySample)
  # 新增 AOT PoC，演示通过编译期注册句柄进行调用
  nova_add_language_sample(Language.BitOpsSample Samples/BitOpsSample)
  nova_add_language_sample(Language.ConcurrencySample Samples/ConcurrencySample)
  nova_add_language_sample(Language.ConfigSample Samples/ConfigSample)
  nova_add_language_sample(Language.TypesSample Samples/TypesSample)
  nova_add_language_sample(Language.ConceptsSample Samples/ConceptsSample)
  nova_add_language_sample(Language.UtilitySample Samples/UtilitySample)
  nova_add_language_sample(Language.InPlaceTagSample Samples/InPlaceTagSample)
  nova_add_language_sample(Language.TypeIDSample Samples/TypeIDSample)
  nova_add_language_sample(Language.TypeTagSample Samples/TypeTagSample)
  nova_add_language_sample(Language.TypeListSample Samples/TypeListSample)
  nova_add_language_sample(Language.TypeMetaSample Samples/TypeMetaSample)
  nova_add_language_sample(Language.TagInvokeSample Samples/TagInvokeSample)
  nova_add_language_sample(Language.HashSample Samples/HashSample)
  nova_add_language_sample(Language.ArithmeticSample Samples/ArithmeticSample)
  nova_add_language_sample(Language.PolicyBuilderSample Samples/PolicyBuilderSample)

  # Reflection + CPO sample
  nova_add_language_sample(Language.ReflectionCpoSample Samples/ReflectionCpoSample)

  # Via() 链式调用机制示例
  nova_add_language_sample(Language.ViaSample Samples/ViaSample)

  # Meta.Generators + Reflection 集成示例
  nova_add_language_sample(Language.MetaGeneratorsSample Samples/MetaGeneratorsSample)

  # Capabilities 概念能力验证样例
  nova_add_language_sample(Language.CapabilitiesCheck Samples/CapabilitiesCheck)

  # Orthogonal Policies 策略组合与契约判定样例
  nova_add_language_sample(Language.OrthogonalPolicies Samples/OrthogonalPolicies)

  # ConversionKind 转换代数示例（分类与 TryFrom）
  nova_add_language_sample(Language.ConversionKind Samples/ConversionKind)

  # PolicySet PoC（阶段一验证样例）
  nova_add_language_sample(Language.PolicySetPoC Samples/PolicySetPoC)

  # StageZero 能力与统一骨架静态断言样例
  nova_add_language_sample(Language.StageZeroCaps Samples/StageZeroCaps)

  # StageM3 Policy selection via Category tags
  nova_add_language_sample(Language.StageM3PolicySelect Samples/StageM3PolicySelect)

  # StageM2 Variant+Monostate Optional/Result helpers
  nova_add_language_sample(Language.StageM2VariantOps Samples/StageM2VariantOps)
  # Host 物理形态容器样例
  nova_add_language_sample(Language.HostContainersSample Samples/HostContainersSample)
  # Optional/Result CPO 演示样例
  nova_add_language_sample(Language.OptionalResultSample Samples/OptionalResultSample)
  # CPO Chain 点链式调用样例
  nova_add_language_sample(Language.CPOChainSample Samples/CPOChainSample)
  # Algorithms 策略化插桩样例
  nova_add_language_sample(Language.AlgorithmsInstrumentationSample Samples/AlgorithmsInstrumentationSample)
  # Optional 概念验证样例（Data.Optional）
  # OptionalPoC is test-only now
  # Iterable 概念验证样例
  # Disabled legacy samples relying on hand-written Flow IR generation
  nova_add_language_sample(Language.OptionalTests Samples/OptionalTests)
  nova_add_language_sample(Language.IterableTests Samples/IterableTests)
  nova_add_language_sample(Language.QueueSample Samples/Queue)
  nova_add_language_sample(Language.TaskManagerSample Samples/TaskManager)
  nova_add_language_sample(Language.VectorSample Samples/Vector)
  nova_add_language_sample(Language.BlockListSample Samples/BlockList)
endif()


## --- Stage E: Contract compliance & CPO checks (API samples) ---
## Disabled by default since Language(API) target is not present
if(NOVA_BUILD_LANGUAGE_SAMPLES)
  nova_add_language_api_sample(Language.AllocatorFails Samples/AllocatorFails)
  nova_add_language_api_sample(Language.AllocatorStats Samples/AllocatorStats)
  nova_add_language_api_sample(Language.CpoContractChecks Samples/CpoContractChecks)
endif()

## --- Stage F: Container Skeleton (State/Storage/Handle/Policy) ---
if(NOVA_BUILD_LANGUAGE_SAMPLES)
  nova_add_language_api_sample(Language.ContainerSkeletonSample Samples/ContainerSkeleton)
endif()


add_subdirectory(SubModules/IR)
add_subdirectory(SubModules/Builder)
add_subdirectory(SubModules/Execution)
add_subdirectory(SubModules/Utiles)

add_executable(IRSystem.Samples)
set_target_properties(IRSystem.Samples PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_sources(IRSystem.Samples PRIVATE 
  Samples/Main.cpp
  Interface/Foundation.IRSystem.ixx
  Interface/Foundation.IRSystem.IR.ixx
  Interface/Foundation.IRSystem.Builder.ixx
  Interface/Foundation.IRSystem.Execution.ixx
  Interface/Foundation.IRSystem.Utiles.ixx
)
target_link_libraries(IRSystem.Samples PRIVATE IRSystem.IR IRSystem.Builder IRSystem.Execution IRSystem.Utiles)



