
### **1. `Eng` (Engine Runtime) 层解耦分析**

**现状痛点**：
*   `Eng.Renderer` 承担了从 RHI（图形API封装）到 渲染管线（Render Pipeline）再到 场景渲染（Scene Rendering）的所有职责。
*   `Eng.Gameplay` 定义模糊，既包含 ECS 数据结构，又包含游戏逻辑框架。
*   `Eng.Runtime` 命名与其所在的层级 `Eng` 语义重叠，职责不清。

**解耦思路讨论**：

#### **A. 渲染系统的垂直切分 (The Rendering Stack)**
渲染是引擎最复杂的子系统，建议将其拆分为三个层级：
1.  **硬件抽象层 (RHI)**：仅负责向 GPU 提交指令，不关心具体渲染什么。
2.  **渲染核心层 (RenderCore)**：负责渲染图（Render Graph）、Pass 管理、资源生命周期。
3.  **图形特性层 (Graphics)**：负责具体的光照、阴影、后处理算法实现。

*   **拟议模块**：
    *   `Eng.RHI`：Vulkan/DX12 的统一抽象，CommandList, Fence, DescriptorHeap。
    *   `Eng.RenderCore`：RenderGraph, ShaderManager, PipelineState。
    *   `Eng.Graphics`：Lights, Shadows, PostProcess, Materials, GI。

#### **B. 游戏性与世界的切分 (World & Gameplay)**
需要区分“数据的容器”和“数据的逻辑”。
1.  **实体组件系统 (ECS/Object)**：纯粹的数据管理。
2.  **世界模拟 (World Simulation)**：负责 Tick 循环、空间查询。
3.  **游戏框架 (Gameplay Framework)**：负责规则、玩家控制、相机控制。

*   **拟议模块**：
    *   `Eng.Entity`：Component, Entity, Archetype (纯数据结构)。
    *   `Eng.World`：Level, SceneGraph, SpatialPartition (Octree/BVH)。
    *   `Eng.Framework`：GameMode, PlayerController, CameraSystem (业务逻辑基类)。

#### **C. 核心与平台 (Core & Platform)**
`Eng.Runtime` 应该被拆解为更具体的职责。
*   **拟议模块**：
    *   `Eng.Core`：引擎的主循环（Main Loop）、全局单例管理、启动参数解析。
    *   `Eng.Platform`：处理 OS 相关的应用层逻辑（如窗口消息泵与引擎事件的桥接，不同于 `Prm.Window` 的底层创建）。

---

### **2. `Dev` (Developer Tools) 层解耦分析**

**现状痛点**：
*   `Dev.Cooking`, `Dev.Packing`, `Dev.Importer` 职责在“资产处理管线”中界限模糊。
*   缺乏对“构建系统（Build System）”的明确定义（如生成 VS 工程、反射代码生成）。

**解耦思路讨论**：

#### **A. 资产管线 (Asset Pipeline)**
资产处理通常分为三个阶段：源文件导入 -> 中间格式转换 -> 运行时格式烘焙。
1.  **资产处理 (Processing)**：监听文件变动，调用转换器。
2.  **资产构建 (Building)**：具体的转换逻辑（如 PNG 转 Texture, FBX 转 Mesh）。
3.  **资产打包 (Archiving)**：将处理好的资产打包成 PAK/VFS 格式。

*   **拟议模块**：
    *   `Dev.AssetProcessor`：后台服务，负责文件监控、依赖分析、任务分发。
    *   `Dev.AssetBuilder`：具体的编译器集合（TextureCompiler, MeshOptimizer）。
    *   `Dev.Artifacts`：管理构建产物（Metadata, Cache）。

#### **B. 代码与反射工具 (Code Generation)**
`Dev.Coding` 太过笼统。现代引擎通常需要独立的反射代码生成器和项目文件生成器。

*   **拟议模块**：
    *   `Dev.Reflector`：解析 C++ 头文件，生成反射元数据代码（`.gen.cpp`）。
    *   `Dev.ProjectGen`：生成 CMake 或 Visual Studio 解决方案文件。
    *   `Dev.ShaderCompiler`：独立于运行时的 Shader 变体编译与字节码优化工具。

---

### **3. `Edt` (Editor) 层解耦分析**

**现状痛点**：
*   `Edt.GUI` 既包含基础控件，又可能包含布局逻辑。
*   `Edt.Sub-Editors` 是一个杂乱的集合，难以维护。
*   `Edt.Infrastructure` 职责不明。

**解耦思路讨论**：

#### **A. UI 架构的分层 (UI Architecture)**
编辑器 UI 需要严格分层：
1.  **原子控件库**：按钮、滑块、文本框（无业务逻辑）。
2.  **框架层**：Docking 系统、多窗口管理、撤销重做栈。
3.  **业务控件层**：属性面板（Inspector）、资源浏览器（Content Browser）。

*   **拟议模块**：
    *   `Edt.UI`：基础控件库（Widgets, Styling, Font）。
    *   `Edt.Shell`：编辑器的主框架（MainFrame, Docking, MenuBar, StatusBar）。
    *   `Edt.Widgets`：复用的复杂控件（ColorPicker, CurveEditor, FileTree）。

#### **B. 工具与视口 (Tools & Viewports)**
具体的编辑器功能应该模块化，便于插件扩展。

*   **拟议模块**：
    *   `Edt.Viewport`：负责将 `Eng.Renderer` 的输出嵌入到编辑器窗口中，处理编辑器摄像机。
    *   `Edt.Inspector`：基于反射自动生成属性编辑界面的模块。
    *   `Edt.Browser`：资产管理界面。
    *   `Edt.Toolkit`：为特定资产类型（如材质、动画）提供编辑器基类。

---

