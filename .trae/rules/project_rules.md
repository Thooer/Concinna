# 基于Rust的次世代游戏引擎 ——Concinna开发准则

项目阶段：从C++23迁移到Rust中

## **1. 哲学**

确定性：
可控性：
强灵活性：迟封装，保留高度定制性
最高控制力：拒绝标准库的一切重磅功能，除了（图形学API，文件编解码，编程语言交互库，深度学习框架）以外，几乎所有内容自研
第一性原理：如有必要优先创新，否则采纳前沿实现。

面向数据，硬件亲和
可维护性：优先考虑健壮性和后期可扩展性而不是MVP。
## **2. 设计**

引擎运行时：
多线程确定性 + 时空变速、回溯 + 沙盒运行时，完美复现Bug
WASM确定性，Lua快速迭代

编辑器：
基于服务器的多人实时编辑


所有模块遵循统一结构，确保一致性：

```
<ModuleName>/
├── API/                        # (必选)接口，禁止使用伞头文件
├── Impl/                       # (可选)实现目录
│   └── <ImplName>/             #       (可选)接口的实现
├── Sample/                     # (可选)示例与测试
│   └── <SampleName>/           #       (必选)示例/测试
├── SubModule/                  # (可选)嵌套子模块
│   └── <SubmoduleName>/        #       (嵌套)遵循标准结构
├── Docs/                       # (可选)文档
├── README.md                   # 模块设计与使用说明
└── Cargo.toml                  # 模块构建配置
```

## **3 宏观**

本表展示了引擎的垂直分层结构，遵循 **Foundation -> System -> Model -> View/Controller** 的单向依赖原则。

| 领域分类 | 层级名称 (模块前缀) | 核心职责与定位 | 层级依赖关系 |开发程度|
| :--- | :--- | :--- | :--- | :--- |
| **基础设施** | **Language(Lang)** | **编程范式**：提供C++语言扩展、元编程、基础类型定义。 | 部分std的原语 | 已稳定 |
|             | **Primitive(Prm)** | **原语**：对OS API的零成本抽象，提供硬件访问的最小单元。 | `Lang` | 已稳定 |
|             | **Capability(Cap)** | **能力**：无状态的通用算法、数据结构与数学库。 | `Prm`, `Lang` | 部分完成 |
|             | **System(Sys)** | **系统**：管理全局硬件资源（GPU/内存/IO）与核心框架（UI/Task）。 | `Cap`, `Prm`, `Lang` | 部分完成 |
| **引擎业务** | **Simulation(Sim)** | **模拟**：**数据的真理层**。定义世界、实体、物理与逻辑状态，不涉及渲染与IO。 | `Sys`, `Cap`, `Prm`, `Lang` |未开始 |
|             | **Engine(Eng)** | **引擎**：**数据的展示层**。只读/消费 `Sim` 数据，驱动硬件进行渲染与播放。 | `Sim`, `Foundation` |未开始 |
|             | **Dev(Dev)** | **开发**：**数据的加工层**。负责资产编译、烘焙以及对 `Sim` 数据的编辑逻辑。 | `Eng`,`Sim`, `Foundation` |未开始 |
|             | **Editor(Edt)** | **编辑器**：集成开发环境的交互界面壳层。 | 所有模块 |未开始 |
| **应用程序** | **Hub(Hub)** | **启动器**：独立程序，负责引擎分发与项目管理。 | `Foundation` |未开始 |
|             | **Web(Web)** | **服务**：后端服务与前端站点。 | 独立 |未开始 |
| **工作区**   | **Plugins(Plg)** | **扩展**：第三方或官方扩展插件。 | 视插件类型而定 |未开始 |
|             | **Projects(Pjt)** | **游戏**：具体的游戏项目逻辑。 | `Eng`, `Sim`, `Plg` |未开始 |

---

## **4 模块**

本表详细列出了各层级下的模块定义及其推演的依赖关系。
*注：依赖关系采用点分法（如 `Prm.File`），`Lang` 代表 Language 层。*

| 模块全名 | 功能描述 | 核心依赖推演 | 开发状态 |
| :--- | :--- | :--- | :--- |
| `Prm.Time` | 高精度计时器、系统时间。 | `Lang.Element` | 已稳定 |
| `Prm.System` | CPU信息、OS版本、进程管理。 | `Lang.Text` | 已稳定 |
| `Prm.Clipboard` | 系统剪贴板读写操作。 | `Prm.Window` | 已稳定 |
| `Prm.Window` | 操作系统原生窗口创建与管理。 | `Lang.Text` | 已稳定 |
| `Prm.Sync` | 互斥锁、信号量、原子操作封装。 | `Lang.Element` | 已稳定 |
| `Prm.Threading` | 原始线程创建、亲和性设置。 | `Prm.Sync` | 已稳定 |
| `Prm.Socket` | Berkeley Sockets 原始封装。 | `Lang.Element` | 已稳定 |
| `Prm.Memory` | 内存原语。 | `Lang.Semantics` | 已稳定 |
| `Prm.DynamicLibrary` | DLL/SO 加载与符号解析。 | `Prm.File` | 已稳定 |
| `Prm.Debug` | 硬件断点、堆栈回溯、系统错误码。 | `Lang.Text` | 已稳定 |
| `Prm.HID` | 原始HID设备（键盘/鼠标/手柄）读取。 | `Lang.Element` | 已稳定 |
| `Prm.File` | 原始文件句柄操作、内存映射文件。 | `Lang.Text` | 已稳定 |
| `Prm.Audio` | 底层音频设备访问（WASAPI/XAudio2等）。 | `Lang.Element` | 已稳定 |
| `Prm.IO` | 基础输入输出流抽象。 | `Prm.File`  | 已稳定 |
| `Prm.WSI` | 窗口系统与图形API（Vulkan/DX）的粘合层。 | `Prm.Window` | 已稳定 |
| `Prm.SIMD` | SSE/AVX/Neon 指令集封装。 | `Lang.Element` | 已稳定 |
|
| `Cap.Math` | 向量、矩阵、四元数、几何计算。 | `Prm.SIMD` |基本完成|
| `Cap.Memory` | 内存分配策略（线性/池化/堆栈分配器）。 | `Prm.System`, `Prm.Memory` |基本完成|
| `Cap.Containers` | 高性能自定义容器（Vector/Map等）。 | `Cap.Memory`, `Cap.Algorithms` |大部分完成|
| `Cap.Algorithms` | 通用排序、查找、图算法。 | `Lang.Paradigm` | 大部分完成 |
| `Cap.Concurrency` | 线程池、无锁队列、Promise/Future。 | `Prm.Threading`, `Prm.Sync` | 大部分完成 |以上内容迁移完成，以下内容未迁移
| `Cap.Config` | INI/JSON/YAML 解析与配置管理。 | `Cap.IO`, `Cap.Containers` |未开始|
| `Cap.Crypto` | 哈希计算、加解密算法。 | `Cap.Algorithms` |大部分完成|
| `Cap.Debug` | 高级断言、可视化调试辅助。 | `Prm.Debug` |未开始|
| `Cap.Identifier` | GUID/UUID 生成、字符串哈希ID。 | `Cap.Crypto`, `Cap.Random` |未开始|
| `Cap.Stream` | 流抽象（Reader/Writer/Seekable）。 | `Prm.File`, `Cap.Memory` | 部分完成 |
| `Cap.AsyncIO/IO` | IO调度（IOCP/io_uring）。 | `Prm.File`, `Prm.Threading` | 未开始 |
| `Cap.Path` | 路径处理（拼接/规范化/扩展名）。 | `Lang.Text` | 未开始 |
| `Cap.Log` | 结构化日志记录、多端输出。 | `Cap.Stream`, `Prm.Time` |未开始|
| `Cap.Network` | 包处理、连接抽象、可靠UDP协议。 | `Prm.Socket`, `Cap.Memory` |未开始|
| `Cap.Plugin` | 插件加载策略、符号导出规则。 | `Prm.DynamicLibrary` |未开始|
| `Cap.Profiling` | 埋点宏、Tracy集成、性能计数器。 | `Prm.Time` |未开始|
| `Cap.Random` | 各种分布的随机数生成器。 | `Cap.Math` |大部分完成|
| `Cap.Reflection` | 运行时类型信息（RTTI）、属性遍历。 | `Lang.Reflection`, `Cap.Containers` |未开始|
| `Cap.Resource` | 资源引用计数、资源句柄定义。 | `Cap.Identifier`, `Cap.Memory` |未开始|
| `Cap.Serialization` | 二进制/文本序列化框架。 | `Cap.Reflection`, `Cap.Stream` |未开始|
| `Cap.Streaming` | 数据流管线、压缩/解压。 | `Cap.Stream`, `Cap.Memory` |未开始|
| `Cap.Serialization` | 二进制/文本序列化框架。 | `Cap.Reflection`, `Cap.IO` |未开始|
| `Cap.Streaming` | 数据流管线、压缩/解压。 | `Cap.IO`, `Cap.Memory` |未开始|
|
| `Sys.Job` |  | `Cap.Concurrency` |
| `Sys.Test` | 单元和集成测试框架。 | `Cap.Log`, `Cap.Debug` |
| `Sys.HotReload` | 代码与资源的运行时热更新。 | `Sys.VFS`, `Prm.DynamicLibrary` |
| `Sys.IR` | 引擎内部数据的通用中间格式处理。 | `Cap.Serialization` |
| `Sys.Memory` | 全局内存预算控制、GC。 | `Cap.Memory`, `Cap.Profiling` |
| `Sys.Network` | RPC、会话管理、状态同步、NAT穿透。 | `Cap.Network`, `Cap.Serialization` |
| `Sys.Plugin` | 插件生命周期管理、依赖解析。 | `Cap.Plugin`, `Sys.VFS` |
| `Sys.Profiling` | 性能数据聚合服务、远程调试服务。 | `Cap.Profiling`, `Sys.Network` |
| `Sys.ResourceManager` | 资源加载、缓存、卸载、依赖管理。 | `Sys.VFS`, `Sys.Task`, `Cap.Resource` |
| `Sys.RHI` | 图形API的抽象层。 | `Prm.WSI` |
| `Sys.RenderGraph` | 渲染指令的抽象层。 | `Sys.RHI` |
| `Sys.Scripting` | 脚本虚拟机集成、绑定生成。 | `Cap.Reflection`, `Sys.VFS` |
| `Sys.Streaming` | 场景/纹理的流式加载策略管理。 | `Sys.ResourceManager`, `Cap.Streaming` |
| `Sys.Task` | 命令协议、全局任务图调度、帧任务管理、双命令缓冲、任务图管理。 | `Sys.Job`, `Cap.Concurrency` |
| `Sys.VFS` | 统一路径抽象、包内文件访问、挂载点。 | `Cap.Path`, `Cap.Stream`, `Prm.File` |
|
| `Sim.Animation` | 动画曲线数据、骨骼层级定义、混合树数据结构。 | `Sim.Schema` |
| `Sim.Component` | 所有标准组件的数据定义（Transform, MeshFilter, Light, RigidBodyData）。 | `Sim.Schema`, `Cap.Math` |
| `Sim.Input` | 键位映射表 (KeyMap)、输入动作定义数据。 | `Sim.Schema` |
| `Sim.Material` | 材质参数表、Shader 引用关系、属性重写数据。 | `Sim.Schema` |
| `Sim.Scene` | 场景图结构 (Parent-Child)、实体列表、世界环境配置数据。 | `Sim.Component` |
| `Sim.Schema` | **核心协议**。定义 Entity ID、GUID、反射宏、序列化标准。 | `Cap.Reflection`, `Cap.Serialization` |
| `Sim.Script` | 脚本属性的序列化存储、脚本引用关系。 | `Sim.Schema` |
|
| `Eng.Animation` | **行为**。读取 `Sim.Animation`，计算骨骼矩阵，写入 `Sim.Component`。 | `Sim.Animation`, `Cap.Math` |
| `Eng.Audio` | **行为**。读取音频组件，调用 `Sys.Audio` 进行 3D 混音和播放。 | `Sim.Component`, `Sys.Audio` |
| `Eng.Core` | **调度**。主循环 (GameLoop)、模块生命周期管理、时间步进控制。 | `Sys.Task`, `Sys.Time` |
| `Eng.Physics` | **行为**。读取 `Sim` 中的刚体数据，驱动 PhysX/Jolt 步进，回写坐标到 `Sim`。 | `Sim.Component`, `Sys.Task` |
| `Eng.Renderer` | **行为**。读取 `Sim.Scene`，构建渲染指令，提交给 `Sys.RHI` 上屏。 | `Sim.Scene`, `Sys.RHI` |
| `Eng.Scripting` | **行为**。加载脚本代码，绑定 `Sim` 数据，执行 `OnUpdate` 逻辑。 | `Sim.Script`, `Sys.Scripting` |
| `Eng.Streaming` | **调度**。根据摄像机位置，动态加载/卸载 `Sim` 资源块。 | `Sim.Scene`, `Sys.ResourceManager` |
|
| `Dev.AssetCompiler` | **格式转换**。FBX -> `Sim.Mesh` (二进制); PNG -> `Sys.Texture` (DDS)。 | `Sim.Schema`, `Thd.Assimp` |
| `Dev.Baker` | **离线计算**。调用 `Eng.Renderer` 渲染光照贴图；调用 `Eng.Physics` 生成碰撞体。 | `Eng.Renderer`, `Eng.Physics` |
| `Dev.EditorCore` | **数据操作**。对 `Sim` 数据的增删改查，实现 Undo/Redo 命令系统。 | `Sim.Scene`, `Dev.AssetCompiler` |
| `Dev.Recorder` | **离线渲染**。**你的需求实现**。驱动 `Eng` 运行指定帧，捕获 Buffer 编码为 MP4/AVI。 | `Eng.Core`, `Eng.Renderer`, `Thd.FFmpeg` |
| `Dev.Server` | **后台服务**。多用户协同编辑时的状态同步服务、资产服务器。 | `Sys.Network` |
| `Dev.Virtualization` | **预处理**。将超大模型切片为 Nanite 格式，将大纹理切片为 VT。 | `Eng.Renderer` (ComputeShader) |
|
| `Edt.Framework` | **UI 骨架**。基于 `Sys.UI` 的 Docking 系统、菜单栏、快捷键管理。 | `Sys.UI`, `Prm.Window` |
| `Edt.Gizmo` | **视口交互**。在 `Eng.Renderer` 之上绘制操作轴，处理鼠标拾取 (Picking)。 | `Eng.Renderer`, `Dev.EditorCore` |
| `Edt.Panels` | **工具面板**。属性检视器 (Inspector)、资源浏览器、控制台。 | `Edt.Framework`, `Dev.EditorCore` |
| `Edt.Sequencer` | **动画制作UI**。时间轴编辑界面，调用 `Dev.Recorder` 导出视频。 | `Edt.Framework`, `Sim.Animation` |
| `Edt.Viewport` | **渲染窗口**。嵌入 `Eng.Renderer` 的输出结果，实现“所见即所得”。 | `Eng.Renderer`, `Sys.UI` |
|
| `Hub.Core` | 引擎安装、版本切换、环境检测。 | 独立技术栈 |
| `Hub.Launcher` | **独立程序**。引擎版本管理、项目创建向导、商城资源下载。 | `Sys.UI`, `Sys.Network` |
| `Hub.Project` | **环境配置**。管理 `.concinna` 项目描述文件、关联引擎 SDK 路径。 | `Cap.Config`, `Sys.VFS` |
|
| `Web.Backend` | 账户系统、资产商店API。 | 独立技术栈 |
| `Web.Frontend` | 官网、文档站、Dashboard。 | 独立技术栈 |
|
| `Thd.Tracy` | 实时性能可视化工具。 |  |


## **5. 代码规范**

### **5.2 命名规范**
- **类型/方法**：`PascalCase`，常量`CONSTANT`，局部变量`kCONSTANT`
- **成员变量**：公开`m_`，私有`t_`，接口`I`，模板`T`
- **异步接口**：异步`Async`，线程安全`Safe`，非阻塞`Try`
- **命名空间**：最多一层，如`Prm::xxx`
- **模块前缀**：和命名空间一致，最多一层，如`Prm.Memory`

### **5.3 模块导入**
- **模块化导入**:仅使用 `import <Module>`，禁止 `#include`
- **标准库隔离**:Lang，Prm以外的模块禁止使用，测试文件除外

### **5.4 模块开发流程**

- **确定模块定位**
- **确定依赖关系**
- **编写API**：尽可能一个结构对应一个文件
- **编写README.md**
- **编写测试**
- **后端实现**