
### **1. 核心痛点分析 (Why Decouple?)**

在现有的 `Eng` -> `Dev` -> `Edt` 结构中，存在几个典型的耦合陷阱，我们需要在重构中解决：

1.  **渲染与逻辑的耦合**：`Eng.Renderer` 往往既包含底层的 RHI（Vulkan/DX12 指令），又包含高层的渲染管线（延迟渲染/前向渲染），甚至包含场景遍历逻辑。这使得替换图形 API 或自定义渲染管线变得极其困难。
2.  **模拟与玩法的耦合**：物理（Physics）、动画（Animation）通常被视为“游戏逻辑”的一部分。但实际上，它们是客观的“模拟（Simulation）”，应该独立于“游戏规则（Gameplay）”存在。编辑器视口也需要物理和动画，但不需要游戏规则。
3.  **资产处理的运行时/离线界限模糊**：`Dev` 层负责资源烘焙，但 `Eng` 层负责资源加载。中间缺乏一个共享的“资产互操作层”。编辑器需要读取原始资产（FBX），运行时读取烘焙资产（PAK），这两者的中间态处理往往导致代码重复。
4.  **UI 与 编辑器逻辑耦合**：`Edt.GUI` 往往和 `Edt.Controller` 绑死。理想情况下，UI 库应该是独立的，编辑器逻辑（如选中物体、Gizmo 操作）应该是无 UI 的纯逻辑，最后才是编辑器应用层将两者结合。

---

### **2. 拆解思路讨论 (Divergent Thinking)**

我们尝试引入新的层级命名空间，将原有的三个层级打散。以下是基于业界前沿实践的拆解方向：

#### **方向 A：图形栈的垂直分层 (The Graphics Stack)**

目前的 `Eng.Renderer` 太重了。图形学在引擎中是第一公民，值得拥有独立的层级。

*   **RHI (Render Hardware Interface)**：
    *   **定位**：纯粹的 API 包装，不包含任何渲染策略。
    *   **职责**：CommandList, Buffer, Texture, Fence, SwapChain。
    *   **讨论**：是否应该将其从 `Eng` 中剥离，成为一个独立于 Gameplay 的底层？这样计算着色器（Compute Shader）任务可以在非游戏应用（如烘焙工具）中独立运行。
*   **Rnd (Rendering Core)**：
    *   **定位**：渲染图（Render Graph）与资源管理。
    *   **职责**：FrameGraph, Pass Management, Shader Resource Binding。它不知道“场景”的存在，只知道“绘制指令”。
*   **Scn (Scene Representation)**：
    *   **定位**：场景数据的视觉表达。
    *   **职责**：MeshComponent, LightComponent, Camera。这是连接 Gameplay 和 Rendering 的桥梁。

#### **方向 B：模拟与框架的分离 (Simulation vs Framework)**

目前的 `Eng.Gameplay` 和 `Eng.Physics` 混在一起。我们需要区分“客观规律”和“主观规则”。

*   **Sim (Simulation)**：
    *   **定位**：不依赖游戏逻辑的客观模拟系统。
    *   **模块**：`Sim.Physics` (物理), `Sim.Animation` (动画), `Sim.Audio` (音频), `Sim.Navigation` (导航)。
    *   **特点**：这些模块应该是“无状态”的或者“数据驱动”的。编辑器视口可以直接驱动 `Sim` 层，而无需启动整个游戏流程。
*   **Fwk (Framework)**：
    *   **定位**：游戏对象生命周期与规则。
    *   **模块**：`Fwk.ECS` (实体组件系统), `Fwk.World` (世界管理), `Fwk.Actor` (对象模型)。
    *   **职责**：这是传统意义上的“Gameplay”层，负责组装 `Sim` 层的功能。

#### **方向 C：资产管线的细化 (Asset Pipeline)**

`Dev` 层目前包含了编译器和打包器。我们需要区分“工具库”和“工具应用”。

*   **Ast (Asset Core)**：
    *   **定位**：资产的中间表示与处理算法。
    *   **职责**：MeshOptimizer, TextureCompressor, ShaderCompiler。
    *   **特点**：这是一组纯算法库，既可以被离线打包工具调用，也可以被编辑器实时调用（例如导入资源时）。
*   **Tls (Tooling Services)**：
    *   **定位**：构建系统与服务。
    *   **职责**：AssetRegistry (资产注册表), BuildSystem (构建依赖图), HotReloadService (热重载服务)。

#### **方向 D：编辑器的 MVC 分离 (Editor Architecture)**

`Edt` 层往往是面条代码的重灾区。

*   **Gui (User Interface)**：
    *   **定位**：纯粹的 UI 控件库（类似 Slate 或 ImGui 的封装）。
    *   **职责**：Button, Panel, DockingSystem, Styling。不包含任何业务逻辑。
*   **Vpt (Viewport & Interaction)**：
    *   **定位**：3D 交互逻辑。
    *   **职责**：Gizmo (坐标轴), Selection (拾取), CameraControl (漫游)。这部分逻辑应该独立于 UI 存在。
*   **Wks (Workspace / Editor App)**：
    *   **定位**：真正的编辑器应用程序。
    *   **职责**：将 `Gui`, `Vpt`, `Fwk` 组装在一起，管理 Project 文件，管理插件加载。

---

### **3. 潜在的新架构层级推演**

基于上述讨论，我们可能会将原有的 `Eng`, `Dev`, `Edt` 扩展为以下 7-8 个层级（命名空间）：

1.  **`Rhi` (Render Hardware Interface)**: 硬件抽象层，极度底层。
2.  **`Sim` (Simulation)**: 物理、动画、音频等独立模拟系统。
3.  **`Rnd` (Rendering)**: 渲染管线、渲染图、材质系统（不含 RHI）。
4.  **`Fwk` (Framework)**: 游戏循环、世界、实体、组件、网络同步（Gameplay 核心）。
5.  **`Ast` (Asset)**: 资产导入、编译、优化算法库（原 Dev 的核心算法）。
6.  **`Tls` (Tools)**: 资产数据库、构建管线、IPC 通信（原 Dev 的服务部分）。
7.  **`Gui` (Interface)**: UI 框架、控件库、样式表。
8.  **`Edt` (Editor)**: 具体的编辑器面板实现、工作流逻辑。

### **4. 待讨论的关键问题**

在正式确定架构之前，我们需要对以下问题进行深入思考：

*   **Q1: 渲染层级的位置**
    `Rnd` 层应该依赖 `Sim` 层吗？（例如渲染需要读取骨骼动画数据）。还是说 `Fwk` 层负责将 `Sim` 的数据拷贝给 `Rnd`？
    *   *思考方向*：为了极致性能，渲染层通常需要直接访问模拟数据，但这破坏了分层。是否引入一个 `Sys.RenderProxy` 中间层？

*   **Q2: 脚本系统的位置**
    脚本（Scripting）通常横跨所有层级。它应该在 `Sys` 层，还是在 `Fwk` 层？
    *   *思考方向*：如果脚本仅用于 Gameplay，放 `Fwk`。如果脚本可以驱动编辑器扩展，放 `Sys` 或 `Cap` 可能更合适。

*   **Q3: 资源定义的归属**
    一个纹理（Texture）的定义，在 `Rhi` 层有一个句柄，在 `Rnd` 层有一个描述符，在 `Ast` 层有一个源文件结构，在 `Fwk` 层有一个资源资产。
    *   *思考方向*：如何避免定义四遍？是否需要一个跨层的 `Shared.Types`？（但这违反了单向依赖）。

*   **Q4: 插件系统的粒度**
    现在的 `Plg` 是顶层。但如果我写了一个新的渲染后端（Vulkan），它应该是一个插件吗？如果我写了一个新的物理引擎（Jolt），它是一个插件吗？
    *   *思考方向*：模块化架构应该允许核心层级本身就是由“内置插件”构成的。
