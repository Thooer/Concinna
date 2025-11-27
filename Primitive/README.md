# Primitive 层架构

## 1. 定位

- 职责：面向操作系统与硬件的原语封装（线程、时间、窗口、IO、Socket、系统拓扑、内存所有权、SIMD 等），为上层能力与引擎提供稳定、可移植的最小集合。
- 边界：不做复杂聚合/策略，不承载业务逻辑；不使用伞头跨模块聚合；仅通过模块化 `import` 暴露接口；尽量不依赖外部库（除极少数图形/编解码等必要场景）。
- 核心设计原则：
  - 移动优先与 RAII，严禁资源泄漏
  - 强类型与统一错误返回（`Flow::Status/Expect`）
  - 性能友好（`noexcept` 默认、值返回优化、完美转发）
  - 接口/实现分离：`API`（对外接口）与 `Impl`（后端，如 `Windows`/`Noop`）
  - 模块内分区：以 `:Types`/`:Ops`/领域分区组织接口，外部透明

## 2. 设计

- 模块结构（示例）：
  - Time：`Types`（`TimePoint/Duration` 与转换）、`Clock`（时间源/`Now`/`Sleep`）、`Utils`（`Stopwatch/Timer/WallClock`）
  - Threading：`Types`、`ThreadSync`（线程/互斥/事件/等待）、`Fiber`（纤程上下文与切换）
  - Window：`Types`（句柄/描述/回调）、`Ops`（创建/销毁/标题/尺寸/光标/消息泵）
  - Socket：`Types`（句柄/协议/地址）、`Ops`（创建/连接/收发/属性）
  - IO：`FileSystem` 与请求模型（`AsyncRequest` 基于 Overlapped/IOCP）、同步/异步读写与映射
  - Ownership：内存所有权与平台分配封装
  - System：系统原语（拓扑、句柄与系统调用桥接）
  - SIMD：向量化指令抽象（供 `Capability.Math` 使用）
  - 其他：Audio/Clipboard/Debug/DynamicLibrary/WSI 等

- 模块之间依赖关系：
  - 所有模块依赖 `Language` 基座（类型/概念/错误模型/文本）
  - `Threading` 依赖 `System/Time`（组亲和/睡眠/计时）
  - `IO` 依赖 `Threading`/`System`（IOCP/句柄），并为 `Capability.IO/Network` 提供请求原语
  - `Socket` 面向 `Capability.Network`，与 `System`/`Threading` 协作（非阻塞/事件）
  - `Window` 为输入与 GUI 上层提供消息泵与事件源

- 后端与构建：
  - 后端位于 `Impl/Windows`（真实实现）与 `Impl/Noop`（占位/可测试）
  - Windows 平台采用 Visual Studio 生成器；统一 `CMakeLists.txt` 管理接口、实现与样例
