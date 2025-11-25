# **游戏引擎项目文件结构 (最终修订版 v3)**

本文档定义了一个现代化、高可扩展性游戏引擎的最终项目文件结构。该结构融合了业界最佳实践，旨在实现高度模块化、清晰的依赖关系和高效的开发流程。整个项目被划分为几个核心支柱（`Engine`, `Projects`, `Plugins`），并辅以支持性的工具链和内容管线。

---

## **二、 引擎源码结构 (`Engine/Source/`)**

引擎源码遵循严格的分层依赖关系，确保底层模块的稳定与高复用性。
```
Engine/Source/                      # 源码根目录 (统一管理所有代码)
├── Core/                       # 0. 核心基础 (无依赖或仅依赖C++标准库和第三方库, 严格与硬件API无关)
│   ├── Types/                  #   - 基础类型定义
│   │   ├── Primitives/         #     - 基础数据类型 (Int8/16/32/64, Float32/64, Bool, Char)
│   │   ├── Containers/         #     - 容器类型 (Array, Vector, List, Map, Set, Queue, Stack)
│   │   ├── Strings/            #     - 字符串处理 (String, StringView, StringBuilder, Encoding)
│   │   ├── SmartPointers/      #     - 智能指针 (UniquePtr, SharedPtr, WeakPtr, IntrusivePtr)
│   │   ├── Delegates/          #     - 委托系统 (Delegate, MulticastDelegate, Event, Signal)
│   │   ├── Variants/           #     - 变体类型 (Variant, Optional, Result, Any)
│   │   ├── Iterators/          #     - 迭代器 (Forward, Bidirectional, Random Access)
│   │   ├── Ranges/             #     - 范围操作 (Range, View, Filter, Transform)
│   │   └── Serialization/      #     - 序列化类型 (Archive, Serializable, TypeInfo)
│   ├── Memory/                 #   - 底层内存管理
│   │   ├── Allocators/         #     - 内存分配器 (Linear, Stack, Pool, Ring, Buddy, TLSF)
│   │   ├── Tracking/           #     - 内存追踪 (Leak Detection, Usage Statistics, Call Stack)
│   │   ├── Pools/              #     - 对象池 (Fixed Pool, Dynamic Pool, Thread-Safe Pool)
│   │   ├── GarbageCollection/  #     - 垃圾回收 (Mark-Sweep, Generational, Incremental)
│   │   ├── Compression/        #     - 内存压缩 (LZ4, Zlib, Custom Compression)
│   │   ├── Virtual/            #     - 虚拟内存 (Page Management, Memory Mapping, Protection)
│   │   ├── Cache/              #     - 缓存管理 (CPU Cache Optimization, Memory Prefetch)
│   │   └── NUMA/               #     - NUMA支持 (Node Affinity, Memory Locality)
│   ├── Threading/              #   - 底层线程原语
│   │   ├── Primitives/         #     - 线程原语 (Thread, Mutex, Semaphore, Condition Variable)
│   │   ├── Atomics/            #     - 原子操作 (Atomic Types, Memory Ordering, Lock-Free)
│   │   ├── Synchronization/    #     - 同步机制 (RWLock, Barrier, Latch, Event)
│   │   ├── ThreadLocal/        #     - 线程本地存储 (TLS, Thread-Safe Singleton)
│   │   ├── Coroutines/         #     - 协程支持 (Stackful, Stackless, Generator)
│   │   ├── Fibers/             #     - 纤程系统 (User-Mode Scheduling, Context Switching)
│   │   ├── WorkStealing/       #     - 工作窃取 (Queue, Scheduler, Load Balancing)
│   │   └── NUMA/               #     - NUMA线程 (CPU Affinity, Thread Migration)
│   ├── Intrinsics/             #   - CPU指令集封装
│   │   ├── x86/                #     - x86指令集 (SSE, SSE2, SSE3, SSSE3, SSE4.1/4.2)
│   │   ├── x64/                #     - x64指令集 (AVX, AVX2, AVX-512, FMA)
│   │   ├── ARM/                #     - ARM指令集 (NEON, SVE, Helium)
│   │   ├── RISC-V/             #     - RISC-V指令集 (Vector Extension, Packed SIMD)
│   │   ├── Vectorization/      #     - 向量化 (Auto-Vectorization, Manual SIMD)
│   │   ├── Prefetch/           #     - 预取指令 (Data Prefetch, Instruction Prefetch)
│   │   ├── Barriers/           #     - 内存屏障 (Load/Store Barriers, Fence Instructions)
│   │   └── Common/             #     - 通用封装和平台检测
│   ├── Math/                   #   - 独立、高性能的数学库
│   │   ├── Primitives/         #     - 基础数学类型 (Vector2/3/4, Matrix3x3/4x4, Quaternion, Plane, Ray)
│   │   ├── Geometry/           #     - 几何计算 (AABB, OBB, Sphere, Frustum, Polygon, Mesh)
│   │   ├── Transforms/         #     - 变换系统 (Transform, Scale, Rotation, Translation)
│   │   ├── Interpolation/      #     - 插值算法 (Linear, Bezier, Spline, Catmull-Rom)
│   │   ├── Noise/              #     - 噪声生成 (Perlin, Simplex, Worley, Fractal)
│   │   ├── Random/             #     - 随机数生成 (LCG, Mersenne Twister, Xorshift, Halton)
│   │   ├── Statistics/         #     - 统计学函数 (Distribution, Probability, Regression)
│   │   ├── Optimization/       #     - 数值优化 (Newton-Raphson, Gradient Descent, Genetic Algorithm)
│   │   ├── DSP/                #     - 数字信号处理 (FFT, Convolution, Filtering)
│   │   └── Physics/            #     - 物理数学 (Dynamics, Kinematics, Collision Math)
│   ├── Algorithms/             #   - 通用算法库
│   │   ├── Sorting/            #     - 排序算法 (Quick, Merge, Heap, Radix, Tim Sort)
│   │   ├── Searching/          #     - 搜索算法 (Binary, Hash, Tree, Graph Search)
│   │   ├── Hashing/            #     - 哈希算法 (CRC, MD5, SHA, xxHash, City Hash)
│   │   ├── Compression/        #     - 压缩算法 (LZ77, LZ4, Zstd, Huffman, Arithmetic)
│   │   ├── Encryption/         #     - 加密算法 (AES, RSA, ChaCha20, Poly1305)
│   │   ├── GraphAlgorithms/    #     - 图算法 (Dijkstra, A*, Floyd-Warshall, MST)
│   │   ├── StringAlgorithms/   #     - 字符串算法 (KMP, Boyer-Moore, Suffix Array)
│   │   └── GeometryAlgorithms/ #     - 几何算法 (Convex Hull, Triangulation, Clipping)
│   └── Meta/                   #   - 元编程与编译期工具
│       ├── TypeTraits/         #     - 类型特征 (is_same, is_base_of, enable_if, SFINAE)
│       ├── Concepts/           #     - 概念约束 (C++20 Concepts, Custom Concepts)
│       ├── Templates/          #     - 模板元编程 (CRTP, Policy-Based Design, Tag Dispatch)
│       ├── Reflection/         #     - 编译期反射 (Type Information, Member Access)
│       ├── CodeGeneration/     #     - 代码生成 (Macro Systems, Template Instantiation)
│       ├── ConstExpr/          #     - 常量表达式 (Compile-Time Computation, consteval)
│       ├── SFINAE/             #     - SFINAE技术 (Substitution Failure, Expression SFINAE)
│       └── Utilities/          #     - 元编程工具 (Index Sequence, Tuple Utilities)
│   
├── Base/                       # 1. 基础服务 (依赖Core)
│   ├── Logging/                #   - 日志系统
│   │   ├── Core/               #     - 日志核心 (Logger, LogLevel, LogMessage, Formatter)
│   │   ├── Sinks/              #     - 日志输出 (Console, File, Network, Database, Memory)
│   │   ├── Filters/            #     - 日志过滤 (Level Filter, Category Filter, Pattern Filter)
│   │   ├── Formatters/         #     - 日志格式化 (Text, JSON, XML, Binary, Custom)
│   │   ├── Appenders/          #     - 日志追加器 (Async, Buffered, Rotating, Compressed)
│   │   ├── Channels/           #     - 日志通道 (Multi-threaded, Lock-free, Priority-based)
│   │   ├── Structured/         #     - 结构化日志 (Key-Value, Metrics, Tracing, Context)
│   │   └── Remote/             #     - 远程日志 (Syslog, ELK Stack, Cloud Logging)
│   ├── Profiling/              #   - 性能分析与追踪系统
│   │   ├── CPU/                #     - CPU性能分析 (Call Stack, Hot Spots, Flame Graph)
│   │   ├── Memory/             #     - 内存分析 (Allocation Tracking, Leak Detection, Heap Analysis)
│   │   ├── GPU/                #     - GPU性能分析 (Draw Calls, Shader Performance, Memory Usage)
│   │   ├── Network/            #     - 网络分析 (Bandwidth, Latency, Packet Loss, Protocol Analysis)
│   │   ├── IO/                 #     - I/O分析 (File Access, Disk Usage, Read/Write Performance)
│   │   ├── Threading/          #     - 线程分析 (Lock Contention, Thread Utilization, Synchronization)
│   │   ├── Sampling/           #     - 采样分析 (Statistical Sampling, Hardware Counters)
│   │   ├── Instrumentation/    #     - 代码插桩 (Manual Markers, Automatic Instrumentation)
│   │   ├── Visualization/      #     - 可视化 (Timeline, Graphs, Heat Maps, Statistics)
│   │   └── Export/             #     - 数据导出 (Chrome Tracing, Perfetto, Custom Formats)
│   ├── Events/                 #   - 事件分发与处理系统
│   │   ├── Core/               #     - 事件核心 (Event, EventHandler, EventDispatcher)
│   │   ├── Queues/             #     - 事件队列 (Priority Queue, Ring Buffer, Lock-free Queue)
│   │   ├── Dispatching/        #     - 事件分发 (Immediate, Deferred, Threaded, Async)
│   │   ├── Filtering/          #     - 事件过滤 (Type Filter, Priority Filter, Conditional)
│   │   ├── Routing/            #     - 事件路由 (Hierarchical, Pattern-based, Rule-based)
│   │   ├── Serialization/      #     - 事件序列化 (Network Events, Persistent Events)
│   │   ├── Debugging/          #     - 事件调试 (Event Logging, Replay, Visualization)
│   │   └── Performance/        #     - 性能优化 (Event Pooling, Batch Processing, Compression)
│   ├── Platform/               #   - 平台抽象层
│   │   ├── FileSystem/         #     - 文件系统 (File I/O, Directory Operations, Path Handling)
│   │   │   ├── VirtualFS/      #       - 虚拟文件系统 (Archive Support, Overlay, Mounting)
│   │   │   ├── Async/          #       - 异步文件操作 (Async Read/Write, Completion Callbacks)
│   │   │   ├── Monitoring/     #       - 文件监控 (Change Detection, Directory Watching)
│   │   │   ├── Compression/    #       - 文件压缩 (ZIP, 7Z, TAR, Custom Formats)
│   │   │   └── Security/       #       - 文件安全 (Permissions, Encryption, Sandboxing)
│   │   ├── Window/             #     - 窗口系统 (Window Creation, Event Handling, Multi-monitor)
│   │   │   ├── Creation/       #       - 窗口创建 (Native Windows, Fullscreen, Borderless)
│   │   │   ├── Events/         #       - 窗口事件 (Resize, Move, Focus, Close)
│   │   │   ├── Properties/     #       - 窗口属性 (Title, Icon, Transparency, Always On Top)
│   │   │   ├── MultiMonitor/   #       - 多显示器 (Display Enumeration, DPI Awareness)
│   │   │   └── Decorations/    #       - 窗口装饰 (Custom Title Bar, Borders, Shadows)
│   │   ├── System/             #     - 系统信息 (Hardware Info, OS Version, Environment)
│   │   │   ├── Hardware/       #       - 硬件信息 (CPU, GPU, Memory, Storage, Network)
│   │   │   ├── OS/             #       - 操作系统 (Version, Features, Capabilities)
│   │   │   ├── Environment/    #       - 环境变量 (System Variables, User Variables)
│   │   │   ├── Performance/    #       - 系统性能 (CPU Usage, Memory Usage, Disk I/O)
│   │   │   └── Power/          #       - 电源管理 (Battery Status, Power Schemes)
│   │   ├── Process/            #     - 进程管理 (Process Creation, IPC, Child Processes)
│   │   │   ├── Execution/      #       - 进程执行 (Launch, Terminate, Suspend, Resume)
│   │   │   ├── Communication/  #       - 进程通信 (Pipes, Shared Memory, Message Queues)
│   │   │   ├── Monitoring/     #       - 进程监控 (Resource Usage, Status Tracking)
│   │   │   └── Security/       #       - 进程安全 (Privileges, Sandboxing, Isolation)
│   │   ├── Network/            #     - 网络平台 (Socket Abstraction, Network Interfaces)
│   │   │   ├── Sockets/        #       - 套接字 (TCP, UDP, Unix Sockets, Raw Sockets)
│   │   │   ├── Interfaces/     #       - 网络接口 (Interface Enumeration, Configuration)
│   │   │   ├── Protocols/      #       - 协议支持 (HTTP, WebSocket, Custom Protocols)
│   │   │   └── Security/       #       - 网络安全 (TLS, Certificates, Encryption)
│   │   └── Time/               #     - 时间系统 (High Resolution Timer, Time Zones)
│   │       ├── Clocks/         #       - 时钟 (System Clock, Steady Clock, High Resolution)
│   │       ├── Timers/         #       - 定时器 (One-shot, Periodic, Precision Timers)
│   │       ├── TimeZones/      #       - 时区 (UTC, Local Time, Time Zone Conversion)
│   │       └── Synchronization/#       - 时间同步 (NTP, PTP, Custom Sync Protocols)
│   ├── Serialization/          #   - 序列化框架
│   │   ├── Binary/             #     - 二进制序列化 (Compact, Versioned, Endian-aware)
│   │   ├── Text/               #     - 文本序列化 (JSON, XML, YAML, TOML, INI)
│   │   ├── Schema/             #     - 模式定义 (Schema Validation, Evolution, Migration)
│   │   ├── Streaming/          #     - 流式序列化 (Large Data, Progressive Loading)
│   │   ├── Compression/        #     - 压缩序列化 (LZ4, Zstd, Custom Compression)
│   │   ├── Encryption/         #     - 加密序列化 (AES, RSA, Digital Signatures)
│   │   ├── Versioning/         #     - 版本控制 (Backward Compatibility, Migration)
│   │   └── Performance/        #     - 性能优化 (Zero-copy, Memory Mapping, Parallel)
│   ├── Reflection/             #   - 反射系统
│   │   ├── TypeInfo/           #     - 类型信息 (Type Registry, Type Hierarchy, Metadata)
│   │   ├── Properties/         #     - 属性系统 (Property Access, Validation, Binding)
│   │   ├── Methods/            #     - 方法调用 (Dynamic Invocation, Parameter Binding)
│   │   ├── Attributes/         #     - 特性系统 (Custom Attributes, Annotation Processing)
│   │   ├── Serialization/      #     - 反射序列化 (Automatic Serialization, Custom Serializers)
│   │   ├── Scripting/          #     - 脚本绑定 (Automatic Binding Generation, Type Mapping)
│   │   ├── Editor/             #     - 编辑器集成 (Property Editors, Inspector Generation)
│   │   └── Performance/        #     - 性能优化 (Type Caching, Fast Property Access)
│   ├── Configuration/          #   - 配置系统
│   │   ├── Formats/            #     - 配置格式 (INI, JSON, XML, YAML, TOML, Binary)
│   │   ├── Hierarchical/       #     - 层次配置 (Inheritance, Override, Merging)
│   │   ├── Dynamic/            #     - 动态配置 (Hot Reload, Runtime Changes, Validation)
│   │   ├── Remote/             #     - 远程配置 (Cloud Config, Distributed Config)
│   │   ├── Validation/         #     - 配置验证 (Schema Validation, Type Checking)
│   │   ├── Encryption/         #     - 配置加密 (Sensitive Data, Key Management)
│   │   ├── Profiles/           #     - 配置档案 (Development, Production, Testing)
│   │   └── Migration/          #     - 配置迁移 (Version Upgrade, Schema Evolution)
│   ├── TaskSystem/             #   - 高级任务/作业系统
│   │   ├── Jobs/               #     - 作业系统 (Job Queue, Job Dependencies, Job Priorities)
│   │   ├── Fibers/             #     - 纤程调度 (Cooperative Scheduling, Context Switching)
│   │   ├── WorkStealing/       #     - 工作窃取 (Load Balancing, Queue Management)
│   │   ├── Dependencies/       #     - 依赖管理 (Task Graph, Dependency Resolution)
│   │   ├── Scheduling/         #     - 任务调度 (Priority Scheduling, Deadline Scheduling)
│   │   ├── Parallel/           #     - 并行执行 (Fork-Join, Parallel For, Map-Reduce)
│   │   ├── Async/              #     - 异步任务 (Futures, Promises, Async/Await)
│   │   └── Monitoring/         #     - 任务监控 (Progress Tracking, Performance Metrics)
│   ├── Module/                 #   - 模块管理器
│   │   ├── Loading/            #     - 模块加载 (Dynamic Loading, Static Linking, Hot Reload)
│   │   ├── Dependencies/       #     - 依赖管理 (Dependency Resolution, Version Checking)
│   │   ├── Registry/           #     - 模块注册 (Module Discovery, Interface Registration)
│   │   ├── Lifecycle/          #     - 生命周期 (Initialize, Startup, Shutdown, Cleanup)
│   │   ├── Versioning/         #     - 版本管理 (Semantic Versioning, Compatibility)
│   │   ├── Security/           #     - 模块安全 (Code Signing, Sandboxing, Permissions)
│   │   ├── Plugins/            #     - 插件系统 (Plugin Discovery, Plugin API)
│   │   └── Debugging/          #     - 调试支持 (Module Debugging, Symbol Loading)
│   ├── I18N/                   #   - 国际化/本地化系统
│   │   ├── Locales/            #     - 区域设置 (Language, Country, Currency, Number Format)
│   │   ├── StringTables/       #     - 字符串表 (Translation Tables, Pluralization)
│   │   ├── Fonts/              #     - 字体系统 (Unicode Support, Font Fallback, Rendering)
│   │   ├── Input/              #     - 输入法 (IME Support, Text Input, Keyboard Layouts)
│   │   ├── Formatting/         #     - 格式化 (Date/Time, Numbers, Currency, Messages)
│   │   ├── Translation/        #     - 翻译系统 (Translation Memory, Machine Translation)
│   │   ├── ResourceLoading/    #     - 资源加载 (Localized Assets, Fallback Resources)
│   │   └── Testing/            #     - 本地化测试 (Pseudo-localization, Layout Testing)
│   └── ResourceManager/        #   - 底层资源管理器
│       ├── Handles/            #     - 资源句柄 (Weak/Strong References, Handle Validation)
│       ├── Loading/            #     - 资源加载 (Async Loading, Streaming, Dependency Loading)
│       ├── Caching/            #     - 资源缓存 (LRU Cache, Memory Management, Cache Policies)
│       ├── Streaming/          #     - 流式加载 (Level-of-Detail, Progressive Loading)
│       ├── Compression/        #     - 资源压缩 (Asset Compression, Runtime Decompression)
│       ├── Encryption/         #     - 资源加密 (Asset Protection, DRM, License Checking)
│       ├── Versioning/         #     - 版本管理 (Asset Versioning, Dependency Tracking)
│       ├── Monitoring/         #     - 资源监控 (Usage Tracking, Performance Metrics)
│       └── Debugging/          #     - 调试工具 (Resource Browser, Dependency Viewer)
│   
├── HAL/                        # 2. 硬件抽象层 (依赖Base, 负责与具体硬件API交互)
│   ├── RHI/                    #   - 渲染硬件接口
│   │   ├── D3D12/              #     - Direct3D 12 实现 (Windows)
│   │   │   ├── Device/         #       - 设备管理 (Adapter, Device, Factory)
│   │   │   ├── CommandList/    #       - 命令列表 (Graphics, Compute, Copy)
│   │   │   ├── Resources/      #       - 资源管理 (Buffers, Textures, Heaps)
│   │   │   ├── Pipeline/       #       - 管线状态 (PSO, Root Signature, Shaders)
│   │   │   ├── Synchronization/#       - 同步对象 (Fence, Event, Barriers)
│   │   │   ├── Memory/         #       - 内存管理 (Allocation, Residency)
│   │   │   └── Debug/          #       - 调试支持 (Debug Layer, PIX Integration)
│   │   ├── Vulkan/             #     - Vulkan 实现 (跨平台)
│   │   │   ├── Instance/       #       - 实例管理 (Instance, Physical Device)
│   │   │   ├── Device/         #       - 逻辑设备 (Device, Queue, Extensions)
│   │   │   ├── Memory/         #       - 内存管理 (VMA, Memory Types, Allocation)
│   │   │   ├── CommandBuffer/  #       - 命令缓冲 (Primary, Secondary, Recording)
│   │   │   ├── Pipeline/       #       - 管线 (Graphics, Compute, Ray Tracing)
│   │   │   ├── Synchronization/#       - 同步 (Semaphore, Fence, Barrier)
│   │   │   ├── RenderPass/     #       - 渲染通道 (Render Pass, Subpass)
│   │   │   └── Extensions/     #       - 扩展支持 (Ray Tracing, Mesh Shaders)
│   │   ├── Metal/              #     - Metal 实现 (macOS/iOS)
│   │   │   ├── Device/         #       - 设备管理 (MTLDevice, MTLLibrary)
│   │   │   ├── CommandBuffer/  #       - 命令缓冲 (MTLCommandBuffer, Encoder)
│   │   │   ├── Resources/      #       - 资源 (MTLBuffer, MTLTexture, Heap)
│   │   │   ├── Pipeline/       #       - 管线状态 (Render/Compute Pipeline)
│   │   │   ├── Shaders/        #       - 着色器 (MSL, Shader Library)
│   │   │   └── Performance/    #       - 性能工具 (GPU Timeline, Counters)
│   │   ├── OpenGL/             #     - OpenGL 实现 (兼容性支持)
│   │   │   ├── Context/        #       - 上下文管理 (Context Creation, Sharing)
│   │   │   ├── Extensions/     #       - 扩展管理 (Extension Loading, Validation)
│   │   │   ├── State/          #       - 状态管理 (State Caching, Binding)
│   │   │   ├── Resources/      #       - 资源 (VBO, VAO, Textures, FBO)
│   │   │   ├── Shaders/        #       - 着色器 (GLSL, Program Objects)
│   │   │   └── Debug/          #       - 调试 (Debug Output, Error Checking)
│   │   ├── WebGPU/             #     - WebGPU 实现 (Web平台)
│   │   │   ├── Adapter/        #       - 适配器 (Adapter Selection, Limits)
│   │   │   ├── Device/         #       - 设备 (Device Creation, Features)
│   │   │   ├── Resources/      #       - 资源 (Buffer, Texture, Sampler)
│   │   │   ├── Pipeline/       #       - 管线 (Render/Compute Pipeline)
│   │   │   ├── CommandEncoder/ #       - 命令编码器 (Render/Compute Pass)
│   │   │   └── Shaders/        #       - 着色器 (WGSL, Shader Modules)
│   │   ├── Common/             #     - 通用RHI接口
│   │   │   ├── Abstraction/    #       - 抽象层 (Device, Context, Resources)
│   │   │   ├── Validation/     #       - 验证层 (Parameter Validation, Debug)
│   │   │   ├── Conversion/     #       - 格式转换 (Texture Formats, Vertex Formats)
│   │   │   ├── Capabilities/   #       - 能力查询 (Feature Detection, Limits)
│   │   │   ├── Statistics/     #       - 统计信息 (Draw Calls, Memory Usage)
│   │   │   └── Profiling/      #       - 性能分析 (GPU Timing, Markers)
│   │   └── RayTracing/         #     - [修订] 光线追踪硬件原语 (RHI层只提供基础能力)
│   │       ├── Acceleration/   #       - 加速结构 (BLAS, TLAS, Updates)
│   │       ├── Pipeline/       #       - RT管线 (Shader Binding Table, Pipeline)
│   │       └── Shaders/        #       - RT着色器类型 (Ray Generation, Hit, Miss)
│   ├── AHI/                    #   - 音频硬件接口
│   │   ├── XAudio2/            #     - XAudio2 实现 (Windows)
│   │   │   ├── Engine/         #       - 音频引擎 (Engine Creation, Mastering Voice)
│   │   │   ├── Voices/         #       - 音频声音 (Source Voice, Submix Voice)
│   │   │   ├── Effects/        #       - 音频效果 (Reverb, EQ, Compression)
│   │   │   ├── Streaming/      #       - 流式播放 (Buffer Management, Callbacks)
│   │   │   └── 3D/             #       - 3D音频 (X3DAudio, Positional Audio)
│   │   ├── WASAPI/             #     - WASAPI 实现 (Windows)
│   │   │   ├── Device/         #       - 设备管理 (Enumeration, Properties)
│   │   │   ├── Client/         #       - 音频客户端 (Shared/Exclusive Mode)
│   │   │   ├── Capture/        #       - 音频捕获 (Microphone, Loopback)
│   │   │   ├── Render/         #       - 音频渲染 (Playback, Low Latency)
│   │   │   └── Events/         #       - 事件处理 (Device Changes, Notifications)
│   │   ├── OpenAL/             #     - OpenAL 实现 (跨平台)
│   │   │   ├── Context/        #       - 上下文管理 (Device, Context Creation)
│   │   │   ├── Sources/        #       - 音频源 (Source Management, Properties)
│   │   │   ├── Buffers/        #       - 音频缓冲 (Buffer Management, Formats)
│   │   │   ├── Listener/       #       - 听者 (Position, Orientation, Velocity)
│   │   │   ├── Effects/        #       - 音频效果 (EFX, Reverb, Filters)
│   │   │   └── Extensions/     #       - 扩展支持 (HRTF, Soft, Creative)
│   │   ├── CoreAudio/          #     - Core Audio 实现 (macOS/iOS)
│   │   │   ├── AudioUnit/      #       - 音频单元 (AU, Effects, Instruments)
│   │   │   ├── AudioQueue/     #       - 音频队列 (Playback, Recording)
│   │   │   ├── AudioSession/   #       - 音频会话 (iOS Session Management)
│   │   │   ├── MIDI/           #       - MIDI支持 (MIDI I/O, Sequencing)
│   │   │   └── Spatial/        #       - 空间音频 (3D Audio, Binaural)
│   │   ├── ALSA/               #     - ALSA 实现 (Linux)
│   │   │   ├── PCM/            #       - PCM接口 (Playback, Capture)
│   │   │   ├── Control/        #       - 控制接口 (Mixer, Volume)
│   │   │   ├── MIDI/           #       - MIDI接口 (Sequencer, Raw MIDI)
│   │   │   └── Plugins/        #       - 插件支持 (Rate Conversion, Effects)
│   │   ├── WebAudio/           #     - Web Audio 实现 (Web平台)
│   │   │   ├── Context/        #       - 音频上下文 (AudioContext, OfflineContext)
│   │   │   ├── Nodes/          #       - 音频节点 (Source, Effect, Destination)
│   │   │   ├── Processing/     #       - 音频处理 (ScriptProcessor, AudioWorklet)
│   │   │   └── Spatial/        #       - 空间音频 (Panner, Listener)
│   │   └── Common/             #     - 通用AHI接口
│   │       ├── Abstraction/    #       - 抽象层 (Device, Stream, Format)
│   │       ├── Mixing/         #       - 混音 (Multi-channel, Format Conversion)
│   │       ├── DSP/            #       - 数字信号处理 (Filters, Effects, Analysis)
│   │       ├── Compression/    #       - 音频压缩 (MP3, OGG, AAC, FLAC)
│   │       ├── Streaming/      #       - 流式处理 (Buffering, Latency Management)
│   │       └── Profiling/      #       - 性能分析 (Latency, CPU Usage, Dropouts)
│   ├── IHI/                    #   - 输入硬件接口
│   │   ├── Keyboard/           #     - 键盘输入
│   │   │   ├── RawInput/       #       - 原始输入 (Windows Raw Input, Linux evdev)
│   │   │   ├── Layout/         #       - 键盘布局 (QWERTY, AZERTY, Dvorak)
│   │   │   ├── IME/            #       - 输入法 (Text Input, Composition)
│   │   │   ├── Shortcuts/      #       - 快捷键 (Key Combinations, Modifiers)
│   │   │   └── Accessibility/  #       - 辅助功能 (Sticky Keys, Filter Keys)
│   │   ├── Mouse/              #     - 鼠标输入
│   │   │   ├── Movement/       #       - 鼠标移动 (Relative, Absolute, Raw)
│   │   │   ├── Buttons/        #       - 鼠标按键 (Left, Right, Middle, Extra)
│   │   │   ├── Wheel/          #       - 滚轮 (Vertical, Horizontal, Precision)
│   │   │   ├── Cursor/         #       - 光标 (Cursor Management, Custom Cursors)
│   │   │   └── Capture/        #       - 鼠标捕获 (Exclusive Access, Clipping)
│   │   ├── Touch/              #     - 触摸输入
│   │   │   ├── Gestures/       #       - 手势识别 (Tap, Swipe, Pinch, Rotate)
│   │   │   ├── MultiTouch/     #       - 多点触控 (Touch Points, Tracking)
│   │   │   ├── Pressure/       #       - 压力感应 (Force Touch, Pressure Levels)
│   │   │   └── Palm/           #       - 手掌检测 (Palm Rejection, Touch Filtering)
│   │   ├── Gamepad/            #     - 游戏手柄
│   │   │   ├── XInput/         #       - XInput支持 (Xbox Controllers)
│   │   │   ├── DirectInput/    #       - DirectInput支持 (Legacy Controllers)
│   │   │   ├── SDL/            #       - SDL游戏手柄 (跨平台支持)
│   │   │   ├── Haptics/        #       - 触觉反馈 (Vibration, Force Feedback)
│   │   │   ├── Mapping/        #       - 按键映射 (Button Remapping, Profiles)
│   │   │   └── Calibration/    #       - 校准 (Dead Zone, Sensitivity)
│   │   ├── VR/                 #     - VR输入
│   │   │   ├── Controllers/    #       - VR控制器 (6DOF Tracking, Buttons)
│   │   │   ├── Tracking/       #       - 追踪系统 (Inside-out, Outside-in)
│   │   │   ├── Gestures/       #       - 手势识别 (Hand Tracking, Finger Tracking)
│   │   │   └── Haptics/        #       - 触觉反馈 (Haptic Gloves, Force Feedback)
│   │   ├── Motion/             #     - 运动感应
│   │   │   ├── Accelerometer/  #       - 加速度计 (Linear Acceleration)
│   │   │   ├── Gyroscope/      #       - 陀螺仪 (Angular Velocity)
│   │   │   ├── Magnetometer/   #       - 磁力计 (Compass, Orientation)
│   │   │   └── Fusion/         #       - 传感器融合 (Orientation, Motion)
│   │   └── Common/             #     - 通用IHI接口
│   │       ├── Abstraction/    #       - 抽象层 (Device, Event, State)
│   │       ├── Events/         #       - 事件系统 (Input Events, Filtering)
│   │       ├── Mapping/        #       - 输入映射 (Action Mapping, Context)
│   │       ├── Buffering/      #       - 输入缓冲 (Event Queue, Smoothing)
│   │       ├── Prediction/     #       - 输入预测 (Latency Compensation)
│   │       └── Recording/      #       - 输入录制 (Replay, Macros, Testing)
│   ├── NHI/                    #   - 网络硬件接口
│   │   ├── Sockets/            #     - 套接字抽象
│   │   │   ├── TCP/            #       - TCP套接字 (Stream, Connection Management)
│   │   │   ├── UDP/            #       - UDP套接字 (Datagram, Multicast)
│   │   │   ├── Unix/           #       - Unix域套接字 (Local IPC)
│   │   │   ├── Raw/            #       - 原始套接字 (Custom Protocols)
│   │   │   └── Async/          #       - 异步套接字 (Non-blocking, Event-driven)
│   │   ├── Protocols/          #     - 协议实现
│   │   │   ├── HTTP/           #       - HTTP协议 (Client, Server, WebSocket)
│   │   │   ├── TLS/            #       - TLS/SSL (Encryption, Certificates)
│   │   │   ├── QUIC/           #       - QUIC协议 (UDP-based, Low Latency)
│   │   │   ├── Custom/         #       - 自定义协议 (Game Protocols, Binary)
│   │   │   └── P2P/            #       - P2P协议 (NAT Traversal, Hole Punching)
│   │   ├── Discovery/          #     - 网络发现
│   │   │   ├── Broadcast/      #       - 广播发现 (LAN Discovery)
│   │   │   ├── Multicast/      #       - 组播发现 (Service Discovery)
│   │   │   ├── DNS/            #       - DNS解析 (Service Records, mDNS)
│   │   │   └── UPnP/           #       - UPnP (Port Mapping, Device Discovery)
│   │   ├── Security/           #     - 网络安全
│   │   │   ├── Encryption/     #       - 加密 (AES, RSA, Elliptic Curve)
│   │   │   ├── Authentication/ #       - 认证 (OAuth, JWT, Custom Auth)
│   │   │   ├── Firewall/       #       - 防火墙 (Packet Filtering, Rules)
│   │   │   └── AntiCheat/      #       - 反作弊 (Packet Validation, Behavior Analysis)
│   │   ├── Performance/        #     - 网络性能
│   │   │   ├── Compression/    #       - 数据压缩 (LZ4, Zstd, Delta Compression)
│   │   │   ├── Batching/       #       - 数据批处理 (Message Batching, Coalescing)
│   │   │   ├── Prioritization/ #       - 优先级 (QoS, Traffic Shaping)
│   │   │   └── Optimization/   #       - 优化 (Nagle Algorithm, Buffer Tuning)
│   │   └── Monitoring/         #     - 网络监控
│   │       ├── Statistics/     #       - 统计信息 (Bandwidth, Latency, Loss)
│   │       ├── Diagnostics/    #       - 诊断工具 (Ping, Traceroute, Speed Test)
│   │       ├── Logging/        #       - 网络日志 (Packet Logging, Connection Logs)
│   │       └── Alerting/       #       - 告警系统 (Connection Issues, Performance)
│   └── CHI/                    #   - [新增] 计算硬件接口 (封装GPU通用计算)
│       ├── CUDA/               #     - CUDA 实现
│       ├── OpenCL/             #     - OpenCL 实现
│       ├── DirectCompute/      #     - DirectCompute 实现
│       └── Common/             #     - 通用计算接口抽象
│   
├── Framework/                  # 3. 编程框架 (依赖Base/HAL, 定义引擎的开发范式与核心数据结构)
│                                   # [架构理念] 本层定义了“是什么”(What)：即引擎的核心数据模型、状态和接口(如世界、对象、组件)。
│                                   # 具体的“如何做”(How)——即驱动这些数据进行更新和模拟的逻辑——则由 `Systems/` 层实现。
│   ├── Application/            #   - 应用程序框架
│   │   ├── Lifecycle/          #     - 生命周期管理
│   │   │   ├── Startup/        #       - 启动流程 (Initialization, Module Loading)
│   │   │   ├── Shutdown/       #       - 关闭流程 (Cleanup, Resource Release)
│   │   │   ├── Suspend/        #       - 挂起恢复 (Mobile Platform, Background)
│   │   │   └── Crash/          #       - 崩溃处理 (Crash Dumps, Recovery)
│   │   ├── Configuration/      #     - 配置管理
│   │   │   ├── Settings/       #       - 设置系统 (User Preferences, Engine Config)
│   │   │   ├── CommandLine/    #       - 命令行参数 (Parsing, Validation)
│   │   │   ├── Environment/    #       - 环境变量 (Platform-specific, Development)
│   │   │   └── Profiles/       #       - 配置文件 (Debug, Release, Shipping)
│   │   ├── Services/           #     - 应用服务
│   │   │   ├── Localization/   #       - 本地化 (Language, Region, Formatting)
│   │   │   ├── Analytics/      #       - 分析统计 (Usage Metrics, Performance)
│   │   │   ├── Telemetry/      #       - 遥测数据 (Error Reporting, Diagnostics)
│   │   │   └── Updates/        #       - 更新系统 (Auto-update, Patching)
│   │   └── Platform/           #     - 平台抽象
│   │       ├── Desktop/        #       - 桌面平台 (Windows, macOS, Linux)
│   │       ├── Mobile/         #       - 移动平台 (iOS, Android)
│   │       ├── Console/        #       - 游戏机 (PlayStation, Xbox, Nintendo)
│   │       └── Web/            #       - Web平台 (WebGL, WebAssembly)
│   ├── EngineLoop/             #   - 引擎主循环
│   │   ├── Core/               #     - 核心循环
│   │   │   ├── MainLoop/       #       - 主循环 (Frame Timing, Delta Time)
│   │   │   ├── FixedUpdate/    #       - 固定更新 (Physics, Network)
│   │   │   ├── VariableUpdate/ #       - 可变更新 (Rendering, Input)
│   │   │   └── LateUpdate/     #       - 延迟更新 (Camera, UI)
│   │   ├── Scheduling/         #     - 调度系统
│   │   │   ├── TaskScheduler/  #       - 任务调度器 (Job System, Work Stealing)
│   │   │   ├── ThreadPool/     #       - 线程池 (Worker Threads, Load Balancing)
│   │   │   ├── Priorities/     #       - 优先级 (Critical, High, Normal, Low)
│   │   │   └── Dependencies/   #       - 依赖管理 (Task Dependencies, Barriers)
│   │   ├── Timing/             #     - 时间系统
│   │   │   ├── GameTime/       #       - 游戏时间 (Pause, Scale, Dilation)
│   │   │   ├── RealTime/       #       - 真实时间 (Wall Clock, High Resolution)
│   │   │   ├── FrameRate/      #       - 帧率控制 (VSync, Frame Limiting)
│   │   │   └── Profiling/      #       - 时间分析 (Frame Time, System Time)
│   │   └── States/             #     - 循环状态
│   │       ├── Running/        #       - 运行状态 (Normal Execution)
│   │       ├── Paused/         #       - 暂停状态 (Game Pause, Debug Break)
│   │       ├── Loading/        #       - 加载状态 (Level Loading, Asset Streaming)
│   │       └── Transitioning/  #       - 过渡状态 (Scene Changes, Mode Switches)
│   ├── ObjectModel/            #   - 面向对象框架
│   │   ├── Object/             #     - 对象系统
│   │   │   ├── Base/           #       - 基础对象模型
│   │   │   │   ├── Object/     #         - 根对象类 (Reference Counting, RTTI)
│   │   │   │   ├── Interface/  #         - 接口系统 (Pure Virtual, Multiple Inheritance)
│   │   │   │   ├── Factory/    #         - 工厂模式 (Object Creation, Registration)
│   │   │   │   ├── Registry/   #         - 对象注册 (Type Registry, Class Database)
│   │   │   │   └── Serialization/#       - 序列化 (Binary, JSON, XML)
│   │   │   ├── Lifecycle/      #       - 生命周期管理
│   │   │   │   ├── Creation/   #         - 对象创建 (Constructor, Initialization)
│   │   │   │   ├── Destruction/#         - 对象销毁 (Destructor, Cleanup)
│   │   │   │   ├── Reference/  #         - 引用计数 (Smart Pointers, Weak References)
│   │   │   │   ├── GarbageCollection/#   - 垃圾回收 (Mark & Sweep, Generational)
│   │   │   │   └── Pool/       #         - 对象池 (Object Pooling, Reuse)
│   │   │   ├── Reflection/     #       - 反射系统
│   │   │   │   ├── TypeInfo/   #         - 类型信息 (RTTI, Type Traits)
│   │   │   │   ├── Properties/ #         - 属性系统 (Getter/Setter, Metadata)
│   │   │   │   ├── Methods/    #         - 方法调用 (Dynamic Invocation, Parameters)
│   │   │   │   ├── Attributes/ #         - 特性标记 (Annotations, Decorators)
│   │   │   │   └── Introspection/#       - 内省 (Class Hierarchy, Member Enumeration)
│   │   │   └── Memory/         #       - 内存管理
│   │   │       ├── Allocators/ #         - 分配器 (Custom Allocators, Memory Pools)
│   │   │       ├── Tracking/   #         - 内存跟踪 (Leak Detection, Usage Statistics)
│   │   │       ├── Debugging/  #         - 调试支持 (Memory Visualization, Dumps)
│   │   │       └── Optimization/#        - 优化 (Cache-friendly Layout, Alignment)
│   │   ├── Actor/              #     - Actor模型
│   │   │   ├── Core/           #       - 核心Actor系统
│   │   │   │   ├── Actor/      #         - 基础Actor (GameObject, Behavior Container)
│   │   │   │   ├── ActorManager/#        - Actor管理器 (Spawning, Lifecycle)
│   │   │   │   ├── ActorComponent/#      - [修订] Actor组件 (基于统一ComponentSystem的特定封装)
│   │   │   │   ├── ActorSystem/ #        - Actor系统 (Update, Message Handling)
│   │   │   │   └── ActorFactory/#        - Actor工厂 (Creation, Templates)
│   │   │   ├── Behavior/       #       - 行为系统
│   │   │   │   ├── StateMachine/#        - 状态机 (FSM, Hierarchical States)
│   │   │   │   ├── BehaviorTree/#        - 行为树 (AI Logic, Decision Making)
│   │   │   │   ├── Scripting/  #         - 脚本行为 (Lua, C#, Visual Scripting)
│   │   │   │   ├── Events/     #         - 事件处理 (Message Passing, Callbacks)
│   │   │   │   └── Coroutines/ #         - 协程系统 (Async Behavior, Yielding)
│   │   │   ├── Communication/  #       - Actor通信
│   │   │   │   ├── Messaging/  #         - 消息传递 (Actor Model, Mailboxes)
│   │   │   │   ├── RPC/        #         - 远程调用 (Remote Procedure Calls)
│   │   │   │   ├── Events/     #         - 事件广播 (Global Events, Local Events)
│   │   │   │   └── Signals/    #         - 信号系统 (Observer Pattern, Delegates)
│   │   │   ├── Networking/     #       - 网络Actor
│   │   │   │   ├── Replication/#         - 复制系统 (State Synchronization)
│   │   │   │   ├── Authority/  #         - 权威性 (Server Authority, Client Prediction)
│   │   │   │   ├── Ownership/  #         - 所有权 (Player Ownership, Transfer)
│   │   │   │   └── Relevancy/  #         - 相关性 (Interest Management, Culling)
│   │   │   └── Specialization/ #       - 特化Actor
│   │   │       ├── Pawn/       #         - 可控制对象 (Player/AI Controlled)
│   │   │       ├── Character/  #         - 角色 (Movement, Animation, Input)
│   │   │       ├── Vehicle/    #         - 载具 (Physics-based, Complex Movement)
│   │   │       ├── Projectile/ #         - 投射物 (Ballistics, Collision)
│   │   │       ├── Pickup/     #         - 拾取物 (Items, Power-ups, Collectibles)
│   │   │       ├── Trigger/    #         - 触发器 (Area Events, Collision Detection)
│   │   │       ├── Camera/     #         - 相机Actor (View Control, Cinematics)
│   │   │       └── Light/      #         - 光源Actor (Dynamic Lighting, Shadows)
│   │   ├── World/              #     - 世界管理
│   │       ├── Core/           #       - 核心世界系统
│   │       │   ├── World/      #         - 世界定义 (Container, Context, State)
│   │       │   ├── WorldManager/#        - 世界管理器 (Multiple Worlds, Switching)
│   │       │   ├── Scene/      #         - 场景系统 (Scene Graph, Hierarchy)
│   │       │   ├── Level/      #         - 关卡系统 (Level Loading, Transitions)
│   │       │   └── Viewport/   #         - 视口系统 (Camera, Rendering Context)
│   │       ├── Streaming/      #       - 流式加载
│   │       │   ├── LOD/        #         - 细节层次 (Level of Detail, Distance-based)
│   │       │   ├── Culling/    #         - 剔除系统 (Frustum, Occlusion, Distance)
│   │       │   ├── Paging/     #         - 分页系统 (World Chunks, Memory Management)
│   │       │   ├── Background/ #         - 后台加载 (Async Loading, Preloading)
│   │       │   └── Prediction/ #         - 预测加载 (Movement Prediction, Prefetching)
│   │       ├── Partitioning/   #       - 空间分割
│   │       │   ├── Octree/     #         - 八叉树 (3D Spatial Partitioning)
│   │       │   ├── Quadtree/   #         - 四叉树 (2D Spatial Partitioning)
│   │       │   ├── BSP/        #         - BSP树 (Binary Space Partitioning)
│   │       │   ├── Grid/       #         - 网格系统 (Uniform Grid, Hash Grid)
│   │       │   └── Hierarchical/#        - 层次分割 (Multi-level, Adaptive)
│   │       ├── Physics/        #       - 物理世界
│   │       │   ├── Simulation/ #         - 物理模拟 (Rigid Body, Soft Body)
│   │       │   ├── Collision/  #         - 碰撞检测 (Broad Phase, Narrow Phase)
│   │       │   ├── Constraints/#         - 约束系统 (Joints, Springs, Motors)
│   │       │   ├── Materials/  #         - 物理材质 (Friction, Restitution, Density)
│   │       │   └── Forces/     #         - 力场系统 (Gravity, Wind, Magnetic)
│   │       └── Persistence/    #       - 世界持久化
│   │           ├── Serialization/#       - 序列化 (World State, Entity Data)
│   │           ├── Streaming/  #         - 流式存储 (Large Worlds, Chunked Data)
│   │           ├── Versioning/ #         - 版本控制 (Schema Evolution, Migration)
│   │           └── Compression/#         - 数据压缩 (LZ4, Zstd, Custom Formats)
│   ├── DataOrientedModel/      #   - 数据驱动框架 (ECS)
│   │   ├── Entity/             #     - 实体系统
│   │   │   ├── Core/           #       - 核心实体架构
│   │   │   │   ├── Entity/     #         - 实体定义 (ID, Handle, Metadata)
│   │   │   │   ├── EntityManager/#       - 实体管理器 (Creation, Destruction, Lookup)
│   │   │   │   ├── EntityQuery/ #        - 实体查询 (Filter, Sort, Batch Operations)
│   │   │   │   ├── EntitySet/  #         - 实体集合 (Groups, Collections, Iteration)
│   │   │   │   └── EntityGraph/#         - 实体图 (Relationships, Hierarchy)
│   │   │   ├── Identification/ #       - 标识系统
│   │   │   │   ├── ID/         #         - 实体ID (Unique, Persistent, Generation)
│   │   │   │   ├── Handle/     #         - 实体句柄 (Weak References, Validation)
│   │   │   │   ├── GUID/       #         - 全局唯一ID (Cross-session, Networking)
│   │   │   │   └── Tagging/    #         - 标签系统 (Categories, Filtering, Search)
│   │   │   ├── Hierarchy/      #       - 层次结构
│   │   │   │   ├── Parent/     #         - 父子关系 (Tree Structure, Inheritance)
│   │   │   │   ├── Children/   #         - 子实体管理 (Iteration, Batch Operations)
│   │   │   │   ├── Transform/  #         - 变换继承 (Local/World Space, Propagation)
│   │   │   │   └── Prefab/     #         - 预制体 (Templates, Instantiation, Variants)
│   │   │   ├── Lifecycle/      #       - 生命周期
│   │   │   │   ├── Creation/   #         - 实体创建 (Factory, Builder Pattern)
│   │   │   │   ├── Activation/ #         - 激活状态 (Enable/Disable, Visibility)
│   │   │   │   ├── Destruction/#         - 实体销毁 (Cleanup, Dependency Handling)
│   │   │   │   └── Persistence/#         - 持久化 (Save/Load, Streaming)
│   │   │   └── Events/         #       - 实体事件
│   │   │       ├── Creation/   #         - 创建事件 (Entity Born, Component Added)
│   │   │       ├── Destruction/#         - 销毁事件 (Entity Died, Component Removed)
│   │   │       ├── Modification/#        - 修改事件 (Component Changed, State Updated)
│   │   │       └── Query/      #         - 查询事件 (Query Results, Change Detection)
│   │   ├── System/             #     - 系统架构
│   │       ├── Core/           #       - 核心系统架构
│   │       │   ├── System/     #         - 基础系统 (Update, Process, Execute)
│   │       │   ├── SystemManager/#       - 系统管理器 (Registration, Scheduling)
│   │       │   ├── SystemGraph/#         - 系统图 (Dependencies, Execution Order)
│   │       │   ├── SystemGroup/#         - 系统组 (Parallel Execution, Batching)
│   │       │   └── SystemQuery/#         - 系统查询 (Component Access, Filtering)
│   │       ├── Scheduling/     #       - 调度系统
│   │       │   ├── Sequential/ #         - 顺序执行 (Single-threaded, Deterministic)
│   │       │   ├── Parallel/   #         - 并行执行 (Multi-threaded, Job System)
│   │       │   ├── Pipeline/   #         - 管线执行 (Stages, Dependencies)
│   │       │   ├── Priority/   #         - 优先级 (High/Low Priority, Critical Systems)
│   │       │   └── Adaptive/   #         - 自适应 (Load Balancing, Dynamic Scheduling)
│   │       ├── Types/          #       - 系统类型 (由 Systems/ 层的具体系统实现)
│   │       ├── Communication/  #       - 系统通信
│   │       │   ├── Events/     #         - 事件系统 (Publish/Subscribe, Messaging)
│   │       │   ├── Signals/    #         - 信号系统 (Callbacks, Observers)
│   │       │   ├── Blackboard/ #         - 黑板系统 (Shared Data, Global State)
│   │       │   └── MessageQueue/#        - 消息队列 (Async Communication, Buffering)
│   │       └── Profiling/      #       - 系统性能分析
│   │           ├── Timing/     #         - 时间统计 (Execution Time, Frame Time)
│   │           ├── Memory/     #         - 内存分析 (Allocation, Usage Patterns)
│   │           ├── Dependencies/#        - 依赖分析 (System Graph, Bottlenecks)
│   │           └── Optimization/#        - 优化建议 (Parallelization, Caching)
│   ├── ComponentSystem/        #   - [新增] 统一的组件系统 (被ObjectModel和DataOrientedModel共享)
│   │   ├── Core/               #     - 核心组件架构
│   │   │   ├── Component/      #         - 基础组件 (Data-only, Behavior-less)
│   │   │   ├── ComponentManager/#        - 组件管理器 (Storage, Iteration)
│   │   │   ├── ComponentArray/#          - 组件数组 (Dense Storage, Cache-friendly)
│   │   │   ├── Archetype/      #         - 原型系统 (Component Combinations)
│   │   │   └── Query/          #         - 查询系统 (Component Filtering, Iteration)
│   │   ├── Types/              #     - 组件类型定义 (由 Systems/ 层的具体系统定义并注册)
│   │   ├── Storage/            #     - 存储策略
│   │   │   ├── SoA/            #         - 结构数组 (Structure of Arrays)
│   │   │   ├── AoS/            #         - 数组结构 (Array of Structures)
│   │   │   ├── Sparse/         #         - 稀疏存储 (Sparse Set, Handle-based)
│   │   │   ├── Dense/          #         - 密集存储 (Packed Arrays, No Holes)
│   │   │   └── Hybrid/         #         - 混合存储 (Adaptive, Performance-based)
│   │   └── Serialization/      #     - 组件序列化
│   │       ├── Binary/         #         - 二进制序列化 (Fast, Compact)
│   │       ├── JSON/           #         - JSON序列化 (Human-readable, Debug)
│   │       ├── Versioning/     #         - 版本控制 (Schema Evolution, Migration)
│   │       └── Streaming/      #         - 流式序列化 (Large Worlds, Chunking)
│   ├── Scripting/              #   - 脚本系统
│   │   ├── Languages/          #     - 脚本语言支持
│   │   │   ├── Lua/            #       - Lua集成
│   │   │   │   ├── VM/         #         - Lua虚拟机 (LuaJIT, Standard Lua)
│   │   │   │   ├── Binding/    #         - C++绑定 (Sol2, LuaBridge, Manual)
│   │   │   │   ├── Debugging/  #         - 调试支持 (Breakpoints, Stack Trace)
│   │   │   │   ├── Profiling/  #         - 性能分析 (Execution Time, Memory)
│   │   │   │   └── Sandboxing/ #         - 沙箱 (Security, Resource Limits)
│   │   │   ├── Python/         #       - Python集成
│   │   │   │   ├── Embedding/  #         - Python嵌入 (CPython, PyPy)
│   │   │   │   ├── Binding/    #         - C++绑定 (pybind11, Boost.Python)
│   │   │   │   ├── Modules/    #         - 模块系统 (Import, Package Management)
│   │   │   │   ├── GIL/        #         - GIL处理 (Threading, Performance)
│   │   │   │   └── Extensions/ #         - 扩展支持 (NumPy, SciPy Integration)
│   │   │   ├── CSharp/         #       - C#集成
│   │   │   │   ├── Runtime/    #         - .NET运行时 (CoreCLR, Mono)
│   │   │   │   ├── Interop/    #         - 互操作 (P/Invoke, COM Interop)
│   │   │   │   ├── Compilation/#         - 编译 (Roslyn, Dynamic Compilation)
│   │   │   │   ├── Debugging/  #         - 调试支持 (Visual Studio Integration)
│   │   │   │   └── Reflection/ #         - 反射 (Type Information, Dynamic Calls)
│   │   │   ├── JavaScript/     #       - JavaScript集成
│   │   │   │   ├── Engine/     #         - JS引擎 (V8, SpiderMonkey, ChakraCore)
│   │   │   │   ├── Binding/    #         - C++绑定 (Embind, Manual Wrapping)
│   │   │   │   ├── Modules/    #         - 模块系统 (ES6 Modules, CommonJS)
│   │   │   │   ├── Async/      #         - 异步支持 (Promises, Async/Await)
│   │   │   │   └── WebAssembly/#         - WASM支持 (High Performance, Portability)
│   │   │   └── Visual/         #       - 可视化脚本
│   │   │       ├── NodeGraph/  #         - 节点图 (Visual Programming)
│   │   │       ├── StateMachine/#        - 状态机 (Visual State Design)
│   │   │       ├── BehaviorTree/#        - 行为树 (AI Logic Design)
│   │   │       ├── Timeline/   #         - 时间轴 (Sequence, Animation)
│   │   │       └── FlowChart/  #         - 流程图 (Logic Flow, Decision Trees)
│   │   ├── Runtime/            #     - 脚本运行时
│   │   │   ├── Execution/      #       - 执行引擎
│   │   │   │   ├── Interpreter/#         - 解释器 (Direct Execution, Debugging)
│   │   │   │   ├── Compiler/   #         - 编译器 (Bytecode, Native Code)
│   │   │   │   ├── JIT/        #         - 即时编译 (Hot Path Optimization)
│   │   │   │   └── Hybrid/     #         - 混合模式 (Interpreted + Compiled)
│   │   │   ├── Memory/         #       - 内存管理
│   │   │   │   ├── GC/         #         - 垃圾回收 (Mark & Sweep, Generational)
│   │   │   │   ├── Allocation/ #         - 内存分配 (Custom Allocators, Pools)
│   │   │   │   ├── Tracking/   #         - 内存跟踪 (Leak Detection, Profiling)
│   │   │   │   └── Optimization/#        - 优化 (Cache-friendly, Compaction)
│   │   │   ├── Threading/      #       - 多线程支持
│   │   │   │   ├── Coroutines/ #         - 协程 (Cooperative Multitasking)
│   │   │   │   ├── Fibers/     #         - 纤程 (Lightweight Threads)
│   │   │   │   ├── Async/      #         - 异步执行 (Futures, Promises)
│   │   │   │   └── Synchronization/#     - 同步 (Mutexes, Atomic Operations)
│   │   │   └── Security/       #       - 安全性
│   │   │       ├── Sandboxing/ #         - 沙箱 (Resource Limits, API Restrictions)
│   │   │       ├── Validation/ #         - 验证 (Input Validation, Type Checking)
│   │   │       ├── Permissions/#         - 权限 (File Access, Network Access)
│   │   │       └── Auditing/   #         - 审计 (Execution Logging, Security Events)
│   │   ├── Development/        #     - 脚本开发工具
│   │   │   ├── IDE/            #       - 集成开发环境
│   │   │   │   ├── Editor/     #         - 代码编辑器 (Syntax Highlighting, IntelliSense)
│   │   │   │   ├── Debugger/   #         - 调试器 (Breakpoints, Watch, Call Stack)
│   │   │   │   ├── Profiler/   #         - 性能分析器 (CPU, Memory, Call Graph)
│   │   │   │   └── Testing/    #         - 测试工具 (Unit Tests, Integration Tests)
│   │   │   ├── HotReload/      #       - 热重载
│   │   │   │   ├── FileWatcher/#         - 文件监控 (Change Detection, Auto-reload)
│   │   │   │   ├── StatePreservation/#   - 状态保持 (Variable Values, Object State)
│   │   │   │   ├── Incremental/#         - 增量更新 (Partial Recompilation)
│   │   │   │   └── Rollback/   #         - 回滚 (Error Recovery, Safe Fallback)
│   │   │   ├── Documentation/  #       - 文档生成
│   │   │   │   ├── API/        #         - API文档 (Function Signatures, Examples)
│   │   │   │   ├── Comments/   #         - 注释解析 (Docstrings, Annotations)
│   │   │   │   ├── Examples/   #         - 示例代码 (Tutorials, Best Practices)
│   │   │   │   └── Reference/  #         - 参考手册 (Language Features, Libraries)
│   │   │   └── Packaging/      #       - 脚本打包
│   │   │       ├── Modules/    #         - 模块打包 (Dependencies, Versioning)
│   │   │       ├── Distribution/#        - 分发 (Package Repositories, Updates)
│   │   │       ├── Optimization/#        - 优化 (Minification, Dead Code Elimination)
│   │   │       └── Encryption/ #         - 加密 (Source Protection, Obfuscation)
│   │   └── Integration/        #     - 引擎集成
│   │       ├── API/            #       - API绑定
│   │       │   ├── Automatic/  #         - 自动绑定 (Reflection-based, Code Generation)
│   │       │   ├── Manual/     #         - 手动绑定 (Custom Wrappers, Optimized)
│   │       │   ├── Validation/ #         - 验证 (Type Safety, Parameter Checking)
│   │       │   └── Documentation/#       - 文档 (API Reference, Usage Examples)
│   │       ├── Events/         #       - 事件系统集成
│   │       │   ├── Subscription/#        - 事件订阅 (Callbacks, Observers)
│   │       │   ├── Broadcasting/#        - 事件广播 (Global Events, Filtering)
│   │       │   ├── Custom/     #         - 自定义事件 (User-defined, Game-specific)
│   │       │   └── Performance/#         - 性能优化 (Event Batching, Filtering)
│   │       ├── Resources/      #       - 资源访问
│   │       │   ├── Loading/    #         - 资源加载 (Async, Streaming, Caching)
│   │       │   ├── Management/ #         - 资源管理 (Lifetime, Dependencies)
│   │       │   ├── Serialization/#       - 序列化 (Custom Formats, Versioning)
│   │       │   └── Streaming/  #         - 流式访问 (Large Assets, Memory Management)
│   │       └── Networking/     #       - 网络集成
│   │           ├── RPC/        #         - 远程调用 (Client-Server, P2P)
│   │           ├── Replication/#         - 状态复制 (Automatic, Manual)
│   │           ├── Messaging/  #         - 消息传递 (Reliable, Unreliable)
│   │           └── Security/   #         - 网络安全 (Encryption, Authentication)
│   └── GameState/              #   - 游戏状态管理框架
│       ├── Core/               #     - 核心状态系统
│       │   ├── GameState/      #       - 游戏状态 (Menu, Playing, Paused, Loading)
│       │   ├── StateManager/   #       - 状态管理器 (Transitions, Stack, History)
│       │   ├── StateStack/     #       - 状态栈 (Push/Pop, Overlay States)
│       │   └── Transitions/    #       - 状态转换 (Fade, Slide, Custom Effects)
│       ├── GameMode/           #     - 游戏模式
│       │   ├── Core/           #       - 核心游戏模式
│       │   │   ├── GameMode/   #         - 基础游戏模式 (Rules, Objectives)
│       │   │   ├── GameRules/  #         - 游戏规则 (Win Conditions, Scoring)
│       │   │   ├── PlayerState/#         - 玩家状态 (Score, Lives, Inventory)
│       │   │   └── GameSession/#         - 游戏会话 (Match, Round, Tournament)
│       │   ├── Types/          #       - 游戏模式类型
│       │   │   ├── SinglePlayer/#        - 单人模式 (Campaign, Arcade, Practice)
│       │   │   ├── MultiPlayer/#         - 多人模式 (Cooperative, Competitive)
│       │   │   ├── Spectator/  #         - 观战模式 (Camera Control, UI)
│       │   │   └── Tutorial/   #         - 教程模式 (Guided Learning, Tips)
│       │   ├── Networking/     #       - 网络游戏模式
│       │   │   ├── Authority/  #         - 权威性 (Server Authority, Client Prediction)
│       │   │   ├── Replication/#         - 状态复制 (Game State, Player State)
│       │   │   ├── Synchronization/#     - 同步 (Time, Events, Actions)
│       │   │   └── AntiCheat/  #         - 反作弊 (Server Validation, Behavior Analysis)
│       │   └── Persistence/    #       - 持久化
│       │       ├── SaveGame/   #         - 存档系统 (Quick Save, Auto Save, Checkpoints)
│       │       ├── Progress/   #         - 进度跟踪 (Achievements, Statistics)
│       │       ├── Settings/   #         - 设置保存 (User Preferences, Key Bindings)
│       │       └── Cloud/      #         - 云存储 (Cross-platform, Sync)
│       ├── UI/                 #     - 状态UI系统
│       │   ├── Menus/          #       - 菜单系统
│       │   │   ├── MainMenu/   #         - 主菜单 (Start, Options, Quit)
│       │   │   ├── PauseMenu/  #         - 暂停菜单 (Resume, Settings, Exit)
│       │   │   ├── OptionsMenu/#         - 选项菜单 (Graphics, Audio, Controls)
│       │   │   └── LoadingScreen/#       - 加载界面 (Progress, Tips, Background)
│       │   ├── HUD/            #       - 游戏内UI
│       │   │   ├── HealthBar/  #         - 生命值条 (Health, Shield, Armor)
│       │   │   ├── Minimap/    #         - 小地图 (Navigation, Objectives)
│       │   │   ├── Inventory/  #         - 物品栏 (Items, Equipment, Crafting)
│       │   │   └── Chat/       #         - 聊天系统 (Text, Voice, Emotes)
│       │   ├── Dialogs/        #       - 对话系统
│       │   │   ├── MessageBox/ #         - 消息框 (Confirmation, Error, Info)
│       │   │   ├── InputDialog/#         - 输入对话框 (Text Input, Validation)
│       │   │   ├── ProgressDialog/#      - 进度对话框 (Loading, Processing)
│       │   │   └── CustomDialog/#        - 自定义对话框 (Game-specific)
│       │   └── Transitions/    #       - UI转换
│       │       ├── Fade/       #         - 淡入淡出 (Alpha, Color)
│       │       ├── Slide/      #         - 滑动 (Direction, Easing)
│       │       ├── Scale/      #         - 缩放 (Zoom In/Out, Bounce)
│       │       └── Custom/     #         - 自定义转换 (Particle, Shader)
│       └── Events/             #     - 状态事件系统
│           ├── StateEvents/    #       - 状态事件
│           │   ├── Enter/      #         - 进入事件 (State Initialization)
│           │   ├── Exit/       #         - 退出事件 (State Cleanup)
│           │   ├── Update/     #         - 更新事件 (Frame Update, Logic)
│           │   └── Transition/ #         - 转换事件 (State Change, Animation)
│           ├── GameEvents/     #       - 游戏事件
│           │   ├── PlayerJoin/ #         - 玩家加入 (Multiplayer, Lobby)
│           │   ├── PlayerLeave/#         - 玩家离开 (Disconnect, Quit)
│           │   ├── GameStart/  #         - 游戏开始 (Match Start, Round Begin)
│           │   └── GameEnd/    #         - 游戏结束 (Victory, Defeat, Draw)
│           ├── SystemEvents/   #       - 系统事件
│           │   ├── LowMemory/  #         - 内存不足 (Mobile Platform, Cleanup)
│           │   ├── NetworkLoss/#         - 网络断开 (Reconnection, Offline Mode)
│           │   ├── FocusLoss/  #         - 失去焦点 (Pause, Background)
│           │   └── DeviceChange/#        - 设备变化 (Controller, Audio Device)
│           └── CustomEvents/   #       - 自定义事件
│               ├── Achievement/#         - 成就事件 (Unlock, Progress)
│               ├── Tutorial/   #         - 教程事件 (Step Complete, Hint)
│               ├── Economy/    #         - 经济事件 (Purchase, Reward)
│               └── Social/     #         - 社交事件 (Friend Request, Message)
│   
├── Systems/                    # 4. 核心功能系统 (依赖Framework/HAL, 实现具体的模拟和处理逻辑)
│                                   # [架构理念] 本层定义了“如何做”(How)：即实现具体的算法和更新循环，操作并驱动 `Framework` 层定义的数据。
│   ├── Renderer/               #   - 渲染系统 (Render Graph, Pipelines, Culling, Lighting)
│   │   └── PostProcessing/     #     - [新增] 后处理
│   │       └── Denoising/      #       - [新增] 降噪算法实现 (Temporal, Spatial, AI)
│   ├── Physics/                #   - 物理系统 (Rigid/Soft Body, Collision - 内部封装对PhysX, Jolt等第三方库的抽象)
│   ├── Audio/                  #   - 音频系统 (Sources, Effects, Spatialization, Mixing)
│   ├── Input/                  #   - 输入系统 (Action Mapping, Gesture Recognition)
│   ├── Animation/              #   - 动画系统 (StateMachine, IK, Blending, Motion Matching)
│   ├── WorldSystem/                  #   - 世界管理 (Partitioning, Streaming, SceneGraph)
│   ├── AI/                     #   - 人工智能 (BehaviorTree, StateMachine, GOAP, NavMesh)
│   ├── UI/                     #   - UI系统 (Retained/Immediate Mode, Layout, Styling)
│   ├── Networking/             #   - 网络系统 (Transport, Replication, Prediction)
│   ├── VFX/                    #   - 视觉特效系统 (Particles, Ribbons, Decals)
│   ├── PCG/                    #   - 程序化内容生成 (Graph-based, Rule-based)
│   ├── Simulation/             #   - 环境仿真系统 (Time of Day, Ecosystem, Crowd)
│   ├── Security/               #   - 安全系统 (内存安全, 数据加密, 反作弊客户端检测与服务器验证)
│   ├── Gameplay/               #   - 通用游戏性系统 (能力系统-GAS, 装备, 任务)
│   ├── Cinematics/             #   - 影视化系统 (Sequencer, Camera Animation)
│   ├── Accessibility/          #   - 辅助功能系统 (色盲模式, 字幕, UI朗读)
│   ├── ML/                     #   - [修订] 机器学习集成
│   │   ├── Inference/          #     - 推理引擎接口 (ONNX Runtime, TensorFlow Lite)
│   │   ├── Algorithms/         #     - [新增] 常用算法实现 (K-Means, SVM, Neural Networks)
│   │   └── Training/           #     - [可选] 训练接口
│   └── Developer/              #   - 开发者专用系统 (Debug Draw, ImGui集成, 控制台)
│   
├── Services/                   # 5. 在线服务 (依赖Systems/Networking)
│   ├── Identity/               #   - 身份验证与账户
│   ├── PlayerData/             #   - 玩家数据存储 (云存档)
│   ├── Matchmaking/            #   - 匹配服务
│   ├── Leaderboards/           #   - 排行榜
│   ├── Telemetry/              #   - 遥测数据收集与分析
│   ├── LiveOps/                #   - 实时运营 (远程配置, A/B Test)
│   ├── Commerce/               #   - 交易服务 (IAP, DLC, Entitlements)
│   ├── Social/                 #   - 社交服务 (好友, 组队, 公会)
│   └── UGC/                    #   - 用户生成内容服务 (创意工坊接口)
│   
└── Editor/                     # 编辑器代码 (是一个特殊的应用层，依赖所有引擎模块)
    ├── Core/                   #   - 编辑器核心逻辑
    ├── Framework/              #   - 编辑器UI框架, 窗口管理
    ├── Viewports/              #   - 各种视口 (3D, 2D, 蓝图)
    ├── AssetEditors/           #   - 各类资源编辑器 (材质, 动画, 粒子等)
    ├── ToolPanels/             #   - 各类工具面板 (细节, 世界大纲)
    ├── Commands/               #   - 撤销/重做系统
    ├── Gizmos/                 #   - 编辑器场景操作辅助图标
    └── Automation/             #   - 编辑器自动化 (脚本化执行重复任务)
```

---



### 核心支柱作为模块
本项目的三个核心支柱——`Engine`、`Projects` 和 `Plugins`——是对上述模块化标准的具体应用。

*   **`Engine/`**:
    引擎本身被视为一个**顶层元模块（Meta-Module）**。因此，在 `/Engine/` 目录下，必须包含 `CMakeLists.txt`, `README.md`, `Tests/` 等元文件，用于管理整个引擎的构建、测试和文档。

*   **`Projects/` (原Games/)**:
    此目录是一个**容器**，用于存放一个或多个独立的项目。每一个项目（如 `Projects/SampleGame/`）都是一个**完整的模块**，必须遵循上述的模块通用结构，拥有自己的 `CMakeLists.txt`、`Source/`、`Content/` 等。

*   **`Plugins/`**:
    与 `Projects/` 类似，此目录也是一个**容器**，用于存放所有插件。每一个插件（如 `Plugins/ExamplePlugin/`）都是一个**独立的、可分发的模块**，同样必须遵循模块通用结构。