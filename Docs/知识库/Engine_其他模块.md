### **引擎目录结构详解 (`Engine/`)**

本章节详细阐述了 `Engine/` 核心支柱的内部文件结构。`Engine` 不仅仅是源代码的集合，它是一个自包含的、可运行的实体，拥有自己的配置、内置资源、核心插件和构建产物。本结构旨在确保引擎本身的健壮性、可维护性和清晰的边界。

```
/Engine/
├── Engine.descriptor               # [核心新增] 引擎描述符文件 (JSON/XML)
│                                   #   - 定义引擎名称、版本号 (如 1.0.0)。
│                                   #   - 列出所有引擎源码模块 (`Core`, `Renderer`, `Physics` 等) 的加载顺序和依赖关系。
│                                   #   - 指定引擎默认启动的编辑器或工具。
│                                   #   - 管理引擎级别的元数据。
│
├── CMakeLists.txt                  # [工程] 引擎的顶层构建脚本
│                                   #   - 作为引擎元模块的构建入口，负责发现并构建 `Source/` 和 `Plugins/` 下的所有模块。
│                                   #   - 定义引擎范围内的全局编译宏 (如 `WITH_EDITOR`, `ENGINE_VERSION`)。
│                                   #   - 聚合所有引擎模块的测试，并创建一个总的 `test-engine` 目标。
│
├── README.md                       # [文档] 引擎模块的"门面"
│                                   #   - 引擎高层架构概览，引用 `Docs/ArchitectureOverview.md`。
│                                   #   - 快速构建和运行引擎编辑器的指南。
│                                   #   - 核心系统（渲染、物理、AI）的设计哲学简介。
│
├── Binaries/                       # [构建产物] 引擎模块的二进制文件 (被.gitignore)
│   ├── Win64/                      #   - Windows 64位平台
│   │   ├── MyEngineEditor.exe      #       - 引擎编辑器可执行文件
│   │   ├── Core.dll                #       - 各模块的动态链接库
│   │   ├── Renderer.dll
│   │   ├── Core.pdb                #       - 调试符号文件
│   │   └── ...
│   └── Linux/                      #   - Linux平台
│       ├── MyEngineEditor          #       - 引擎编辑器可执行文件
│       ├── libCore.so              #       - 各模块的共享对象库
│       └── ...
│
├── Config/                         # [配置] 引擎层级的默认配置文件
│                                   #   - 为所有项目提供一个基线配置，项目可以继承并覆盖这些配置。
│   ├── DefaultEngine.ini           #   - 核心引擎设置 (主循环、线程池、模块加载)。
│   ├── DefaultInput.ini            #   - 默认输入绑定 (编辑器快捷键、标准游戏手柄布局)。
│   ├── DefaultRender.ini           #   - 默认渲染设置 (默认RHI、特性级别、质量预设)。
│   ├── DefaultPhysics.ini          #   - 默认物理设置 (默认物理引擎、碰撞通道、表面类型)。
│   ├── DefaultEditor.ini           #   - 默认编辑器设置 (布局、视口设置、主题)。
│   └── DefaultLog.ini              #   - 默认日志分类和详细级别。
│
├── Content/                        # [资源] 引擎内置的、已处理过的资源
│                                   #   - 作为引擎功能和编辑器UI的基础，或作为项目的默认/占位资源。
│   ├── Engine/                     #   - 引擎核心内容
│   │   ├── Materials/              #       - 基础材质
│   │   │   ├── M_DefaultLit.uasset #         - 默认PBR光照材质
│   │   │   ├── M_DefaultUnlit.uasset #       - 默认无光材质
│   │   │   └── M_DebugGrid.uasset  #         - 调试网格材质
│   │   ├── Meshes/                 #       - 基础几何体
│   │   │   ├── SM_Cube.uasset      #         - 立方体、球体、平面等
│   │   │   └── SM_Sphere.uasset
│   │   ├── Textures/               #       - 基础纹理
│   │   │   ├── T_DefaultDiffuse.uasset #     - 默认白/灰/黑纹理
│   │   │   ├── T_DefaultNormal.uasset  #     - 默认法线纹理
│   │   │   └── T_Noise_Perlin.uasset #     - 通用噪声纹理
│   │   └── Fonts/                  #       - 默认字体
│   │       └── F_Roboto.uasset     #         - 用于UI和调试文本的字体
│   ├── Editor/                     #   - 编辑器专属内容
│   │   ├── Icons/                  #       - 编辑器UI图标
│   │   │   ├── Icon_Save_32x32.png
│   │   │   └── Icon_Play_32x32.png
│   │   ├── Sounds/                 #       - 编辑器交互音效
│   │   │   ├── CompileSuccess.wav
│   │   │   └── Notification.wav
│   │   └── Themes/                 #       - 编辑器UI主题
│   │       ├── DarkTheme.json
│   │       └── LightTheme.json
│   └── Gizmos/                       #   - 编辑器操作辅助图标(Gizmo)的资源
│       ├── Materials/              #       - Gizmo专用材质 (如半透明、高亮)
│       └── Meshes/                 #       - Gizmo专用模型 (移动、旋转、缩放图标)
│
├── Plugins/                        # [核心插件] 引擎功能的核心组成部分
│                                   #   - 与引擎紧密耦合，默认启用，并与引擎一同分发。
│   ├── Runtime/                    #   - 提供运行时功能的插件
│   │   ├── ImGui/                  #       - ImGui调试UI集成插件
│   │   ├── Physics/                #       - 物理引擎封装
│   │   │   └── JoltPhysics/        #         - (示例) Jolt物理引擎的具体实现
│   │   └── Scripting/              #       - 脚本语言集成
│   │       └── Lua/                #         - (示例) Lua语言支持插件
│   └── Developer/                  #   - 提供开发/编辑器功能的插件
│       ├── VisualStudioTools/      #       - Visual Studio IDE集成工具
│       ├── GitSourceControl/       #       - Git版本控制系统在编辑器中的集成
│       └── RenderDocPlugin/        #       - RenderDoc渲染调试器集成
│
├── Shaders/                        # [着色器] 引擎内置的着色器源码 (.hlsl, .glsl, .wgsl)
│                                   #   - 编译后会成为引擎内容的一部分，但源码在此处维护。
│   ├── Common/                     #   - 通用函数库
│   │   ├── BRDF.hlsl               #       - PBR光照模型实现
│   │   ├── Math.hlsl               #       - 常用数学函数
│   │   └── Packing.hlsl            #       - 数据打包/解包函数
│   ├── Material/                   #   - 表面着色器
│   │   ├── Lit.hlsl                #       - 标准光照着色器
│   │   ├── Unlit.hlsl              #       - 无光着色器
│   │   └── Transparent.hlsl        #       - 透明材质着色器
│   ├── PostProcess/                #   - 后处理效果
│   │   ├── Tonemapping.hlsl        #       - 色调映射
│   │   ├── Bloom.hlsl              #       - 泛光效果
│   │   └── FXAA.hlsl               #       - 抗锯齿
│   ├── Compute/                    #   - 通用计算着色器
│   │   ├── Skinning.compute        #       - GPU蒙皮
│   │   └── ParticleSimulation.compute #    - 粒子模拟
│   ├── RayTracing/                 #   - 光线追踪着色器
│   │   ├── RayGen.rgen             #       - 光线生成
│   │   ├── ClosestHit.rchit        #       - 最近命中
│   │   └── Miss.rmiss              #       - 未命中
│   └── Private/                    #   - 引擎内部使用的着色器 (如用于Gizmo绘制、阴影生成等)
│       ├── ShadowPass.hlsl
│       └── Gizmo.hlsl
│
├── Source/                         # [核心] 引擎C++源码
│
└── Tests/                          # [测试] 引擎级的集成与功能测试
    ├── Integration/                #   - 测试多个核心模块协同工作的场景
    │   └── TestRenderAndPhysics.cpp #      - 例如: 测试渲染系统能否正确绘制物理系统模拟出的对象
    ├── Functional/                 #   - 编辑器功能测试
    │   └── TestEditorAssetImport.py #      - 例如: 自动化测试脚本，导入FBX并验证结果
    └── Performance/                #   - 引擎整体性能基准测试
        └── BenchmarkFullScene.cpp  #       - 渲染一个包含所有特性(物理、动画、VFX)的复杂场景，记录帧率和性能数据。
```

