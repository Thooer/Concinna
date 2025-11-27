# Capability 层架构 README

## 1. 定位

- 职责：在 `Primitive` 原语之上实现通用能力（并发、容器、内存、数学、日志、配置、网络、资源、序列化、性能分析等），面向引擎业务提供高性能、可扩展的逻辑组件。
- 边界：不直接进行系统调用（通过 `Primitive` 间接实现）；不承载长期业务状态（除缓存/池化）；避免平台细节外泄；后端可插拔但对外稳定。
- 核心设计原则：
  - 重逻辑、轻状态；数据结构与策略分离
  - 统一的错误与资源管理（复用 `Language.Flow/Element`）
  - 分区组织与后端抽象（接口 `Interface` + 后端 `Backends`）
  - 性能与可维护性平衡（剖析驱动、模块化扩展）

## 2. 设计

- 模块结构：
  - Concurrency：调度/纤程/队列/任务组/事件/计数器/策略，驱动统一事件源（计时器/IOCP/OS 事件）
  - Containers：矢量/双端队列/环形缓冲/哈希映射/侵入式结构，全部基于统一内存资源
  - Memory：系统/线程缓存/池/帧/栈/NUMA/小对象/调试资源，提供统一 `Allocator` API
  - Math：向量/矩阵/四元数包计算，基于 `Primitive.SIMD`，包含轻量世界坐标（LWC）与存储布局
  - Log：异步日志器（TLS 缓冲 + MPMCQueue + 后台线程），提供批量与字段化输出
  - Profiling：统一函数表与 `Zone` RAII，后端可接入 `ThirdParty/Tracy` 或自研原生
  - Reflection：与 `Language.Reflection` 对接，提供运行时可用的类型注册/字段访问与序列化钩子
  - IO：高层 IO 管线（背压/缓冲/过滤器链），聚合 `Primitive.IO` 的异步请求与 `Concurrency.Driver`
  - Network：在 `Socket`/`IO`/`Concurrency` 上构建协议栈（HTTP/WebSocket/RPC）与会话管理
  - Resource：资源生命周期（加载/卸载/缓存/热重载），统一资源句柄与依赖管理
  - Serialization：二进制/文本编解码、版本化与 Schema 校验，联动 `Reflection` 与 `Text`
  - Debug/Config：调试工具与配置模型，分别为开发期与运行期提供支持

- 模块之间依赖关系：
  - Memory → Containers → Algorithms
  - Concurrency 依赖 `Primitive.Threading/Time/IO` 与 `System`，驱动 `IO` 与调度
  - IO 依赖 `Primitive.IO` 与 Concurrency（请求/事件/背压），向上为 Network/Resource 提供能力
  - Network 依赖 Socket/IO/Concurrency/Serialization
  - Resource 依赖 IO/Serialization/Reflection/Log/Profiling
  - Profiling/Log 为所有模块提供基础诊断能力

- 依赖顺序建议：
  - 先行：Memory/Containers/Algorithms、Math、Profiling/Log
  - 并发：Concurrency（Driver/Scheduler/Fiber/TaskGroup）
  - IO 与网络：IO → Serialization → Network
  - 资源：Reflection → Resource（与 IO/Serialization 整合）
