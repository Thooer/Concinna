

### **1. 现状痛点分析**

在当前的架构中，存在以下显著的耦合风险：

1.  **`Eng` (Engine) 的臃肿**：
    *   `Eng.Renderer`（渲染）通常需要依赖 `Eng.Scene`（场景数据），但 `Eng.Scene` 为了剔除或调试绘制，往往又想引用 `Eng.Renderer` 的类型。
    *   `Eng.Gameplay`（玩法）和 `Eng.Physics`（物理）混在一起，导致纯数据层（Data）与行为层（Behavior）不分。
    *   **后果**：编译时间极长，修改底层渲染可能导致上层玩法代码重编。

2.  **`Dev` (Development) 的定位尴尬**：
    *   `Dev.Compiler`（Shader编译）既是离线工具，也可能在游戏运行时（如Mod支持）被调用。
    *   目前它位于 `Eng` 之上，但实际上它应该为 `Eng` 提供数据格式标准。

3.  **`Edt` (Editor) 的双重职责**：
    *   它既包含通用的 UI 控件库（如 `Edt.GUI`），又包含具体的业务逻辑（如 `Edt.Controller`）。
    *   如果我想做一个独立的“材质查看器”小工具，我被迫引入整个庞大的编辑器依赖。

---

### **2. 架构解耦推演**

为了遵循 **单向依赖** 和 **职责单一** 原则，建议将原有的 3 层扩展为 **6 层**。

#### **新的层级建议 (从底向上)**

| 新层级代号 | 建议命名空间 | 全称 | 核心职责 | 对应旧模块 |
| :--- | :--- | :--- | :--- | :--- |
| **Level 5** | **`Cor`** | **Core (核心层)** | **引擎的微内核**。定义模块管理、运行时循环、基础资源**定义**（非加载）。不包含具体业务逻辑。 | `Eng.Runtime`, `Eng.ModuleSystem`, `Eng.Resource`(Def) |
| **Level 6** | **`Dom`** | **Domain (领域层)** | **纯数据与逻辑框架**。定义 World, Entity, Component, Scene Graph。它是渲染器和物理引擎操作的“对象”。 | `Eng.Scene`, `Eng.Gameplay`(ECS), `Eng.Messaging` |
| **Level 7** | **`Fnc`** | **Function (功能层)** | **具体的子系统实现**。它们是 `Dom` 数据的“处理器”。渲染器、物理模拟、音频混音都在这里。 | `Eng.Renderer`, `Eng.Physics`, `Eng.Audio`, `Eng.Animation` |
| **Level 8** | **`Pip`** | **Pipeline (管线层)** | **资产处理管线**。负责将外部数据转换为 `Cor` 和 `Dom` 定义的内部格式。 | `Dev.*` (Baker, Compiler, Importer) |
| **Level 9** | **`Kit`** | **Toolkit (工具层)** | **编辑器基础设施**。UI 控件库、Gizmo 绘制库、窗口停靠系统。不包含具体编辑器业务。 | `Edt.GUI`, `Edt.Infrastructure` |
| **Level 10** | **`Wks`** | **Workspace (工作区)** | **集成开发环境**。组装 `Fnc`、`Pip` 和 `Kit`，形成最终的编辑器应用。 | `Edt.WorkSpace`, `Edt.Controller`, `Edt.Plugins` |