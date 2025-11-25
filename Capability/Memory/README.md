# Memory 统一设计规范（对标业界顶级分配器）

## 1. 目标与 SLO

- 显式控制：所有分配与释放显式携带 `size` 与 `align`，无隐式堆与全局状态。
- 无异常：以 `Expect/Expect` 传递错误，区分运行时错误与编程错误。
- 可组合：策略与修饰器解耦，按场景拼装（小对象/变长/帧/大页/分区/标签化）。
- 跨模块 ABI 稳定：统一 `IMemoryResource` 契约与 `Allocator` 句柄，模块边界安全多态。
- 性能目标（线程本地稳态）：小对象 P50 < 80ns、P99 < 200ns；帧重置 < 150µs；中等变长碎片率 < 1.5%。

## 2. 设计原则

- Explicit over Implicit：拒绝隐式全局堆；接口参数显式化；窄接口最小暴露。
- No-Exception Guarantee：运行时错误返回码；编程错误断言；`[[nodiscard]]` 强制检查。
- Composition over Inheritance：策略实现与并发/观测修饰器分离，轻量句柄传递。
- Memory as Data：以 `MemoryBlock` 为基本语义单元，便于上层感知真实大小与优化布局。
- 抵制共享可变：线程本地缓存优先；跨线程通过消息/远端释放通道移交所有权。

## 3. 分层架构

- L0 Primitives：`MemoryBlock`、`Alignment` 等基础原语与数学工具。
- L1 Platform：`Platform::Memory::VirtualMemory/Heap`；`Reserve/Commit`、页保护、对齐适配、大页。
- L2 Abstraction：`IMemoryResource` 纯虚接口，ABI 稳定；`IsEqual` 决定跨实例释放合法性。
- L3 Strategies：`Frame/Stack/Pool/SmallObject/TLSF/Partition/HugePage/System/Fallback/Tagged`。
- L4 User Interface：`Allocator` 句柄与对象构造/析构语法糖；只导入模块伞头，禁止跨分区导入。

## 4. 核心接口契约

```cpp
virtual Expect<MemoryBlock> Allocate(USize size, USize align) noexcept;
virtual void Deallocate(MemoryBlock block, USize align) noexcept;
virtual Expect<MemoryBlock> Reallocate(MemoryBlock block, USize newSize, USize align) noexcept;
virtual bool IsEqual(const IMemoryResource& other) const noexcept;
```
- `Allocate`：返回满足对齐且 `block.size >= size` 的 `MemoryBlock`，失败返回具体 `StatusCode`。
- `Deallocate`：调用方必须提供与分配时一致的 `size/align`；允许 O(1) 释放与无 Header 设计。
- `Reallocate`：优先原地扩/缩容；失败时移动扩容并返回新块；旧块不再有效。
- `IsEqual`：等价资源可互相释放；不等价时需通过远端释放通道或原所有者释放。

- `Allocator`：值类型句柄，组合 `Alloc/Free` 与对象 `New/Delete`；
- `OwnedMemoryBlock`：RAII 持有资源引用，作用域结束自动释放；可移动、不可拷贝。

## 5. 策略族（Allocator Suite）

- FrameAllocator（线性/Arena）：按帧生命周期批量 `Reset`；配合 VM 的按需 `Commit` 与 Guard Pages。
- StackAllocator：LIFO 释放，支持 `Marker` 快速回滚；递归/阶段性构造场景。
- PoolAllocator（固定大小）：Free-list；跨线程释放通过无锁队列回收至拥有者线程；缓存亲和。
- SmallObjectAllocator：8–256B 大小类分层；线程本地缓存 + 中央回收器；批量远端释放合并。
- TLSF/Segregated Fits：中等变长分配近似 O(1) 路径；碎片率可控。
- PartitionAllocator：按子系统/对象类型隔离分区，提升局部性与安全性；可配 Guard Pages。
- HugePageArena：在支持平台绑定大页以提升带宽与降低 TLB Miss；自动降级回退。
- SystemAllocator：基于 `Platform::Memory::Heap` 的兜底实现；工具链与非关键路径。
- FallbackChain：策略级联回退（SmallObject → TLSF → System）；失败时顺序尝试。
- TaggedAllocator：所有分配带 `MemoryTag`（Graphics/Audio/Physics/UI/Tooling…），用于观测与预算。

## 6. 并发与远端释放

- 线程本地缓存（TLC）：每线程维护小对象缓存与最近使用块，减少锁争用。
- 远端释放（Remote Free）：跨线程 `Deallocate` 不阻塞分配线程；无锁队列批量回收到拥有者线程。
- 原子与批处理：仅在必要边界使用细粒度原子；合并操作减少 CAS 次数与抖动。
- 等价性与迁移：`IsEqual` 为真则直接跨实例释放；否则通过远端释放迁移所有权。

## 7. 虚拟内存与大页

- 两阶段提交：`Reserve` 与 `Commit` 分离；线性增长的连续 Arena；可设置 `Guard Pages`。
- 延迟提交与预热：按访问模式进行批量或惰性提交，降低页错误与冷启动抖动。
- 大页支持：可选绑定到大页（Windows Large Pages/2MB）；不可用时自动降级。

## 8. NUMA 感知

- 节点绑定：Arena/Pool 可绑定到目标 NUMA 节点；线程本地堆默认绑定本地节点。
- 读多写少：跨节点复制只读结构或分区化分配，降低远程访问延迟。

## 9. 安全与调试

- Canary/Guard Pages：在 Debug 构建填充 Magic Numbers 与设置保护页；捕获越界与 UAF。
- 泄漏检测：退出时输出未释放块与调用栈（与 `StackTrace` 协作）。
- 契约断言：对齐需为 2 的幂、`size/align` 一致性、空指针释放与双重释放在 Debug 断言。

## 10. 可观测与预算

- 指标集合：分配/释放次数、失败码、峰值、活跃字节、碎片率、远端释放队列长度。
- 预算模型：按 `MemoryTag` 的配额与限流；超限返回特定 `StatusCode` 并触发告警事件。
- 采样与事件：向 Profiling 输出周期快照与尖峰事件；支持低开销采样。

## 11. 配置

- 编译期：启用/禁用 Canary/Leak-Detection、大页、默认对齐（如 16/32）。
- 运行时：`MemoryConfig`（大小类阈值、预算、NUMA 策略、采样频率）。

## 12. 使用指南

- 暂存/高频：`FrameAllocator`（每帧 `Reset`）。
- 递归/阶段：`StackAllocator` + `Marker`。
- 大量同尺寸对象：`PoolAllocator` 或 `SmallObjectAllocator`。
- 中等变长：`TLSF/Segregated Fits`。
- 大块连续空间：`HugePageArena` 或 VM Arena。
- 非关键与工具：`SystemAllocator`。

## 13. 业界对标

- Unreal `FMalloc`：可替换全局接口、线程本地堆、帧分配器、Guard Pages、标签化配额。
- EASTL：可注入 `CoreAllocator`、容器与分配器解耦、命名/类别化与结果导向接口。
- PartitionAlloc（Chrome）：大小类分区隔离与安全性提升；近似 O(1) 路径与低碎片。
- mimalloc/rpmalloc：线程本地小对象堆、远端释放回收、延迟提交与大页优化。

## 14. 路线图

- Phase 1：`Frame/Stack/System` 与 `Allocator` 句柄；确定性合同（释放需 `size/align`）。
- Phase 2：`SmallObjectAllocator`（大小类 + TLC + 中央回收器）与 `TLSF`；远端释放通道。
- Phase 3：`PartitionAllocator`、`FallbackChain`、`TaggedAllocator` 与观测/预算集成。
- Phase 4：Canary/Guard、Leak-Detection、NUMA 策略与大页落地，端到端基准与压力测试。

## 15. 术语

- Arena：线性增长的连续内存区域，支持批量重置。
- Size Class：基于大小的类别划分，用于快速查找与复用。
- Remote Free：跨线程释放通过队列批量回收到拥有者线程。
- Partition：逻辑分区，减少跨类别复用、提升局部性与安全性。

## 8. 业界最佳实践对标 (Industry Benchmarks)

- Unreal Engine：全局可替换的 `FMalloc` 接口、线程本地堆、帧分配器（Frame Allocator）、严格的对齐语义与 Guard Pages、带标签的配额管理。
- EA/EASTL：用户可注入的 `CoreAllocator`、类别化（Categories/Tags）与命名（Debug Name）、容器与分配器的解耦、结果导向接口。
- Chrome PartitionAlloc：分区式大小类隔离，减少跨类复用导致的碎片与安全问题，O(1) 近似路径。
- mimalloc / rpmalloc：线程本地小对象堆、远端释放（Remote Free）回收通道、按大小类的分层空闲表、可选大页绑定与延迟提交策略。

本模块在不引入外部依赖前提下，采用“策略组合 + 平台原语”实现上述关键能力。

## 9. 策略集合 (Allocator Suite)

- FrameAllocator（线性/Arena）：按帧生命周期批量 `Reset`，配合虚拟内存的 `Reserve/Commit`，服务高频暂存（解析、组件构建、渲染命令）。
- StackAllocator：LIFO 释放，支持 `Marker` 快速回滚；用于递归/阶段性构造。
- PoolAllocator（固定大小）：Free-list 按块大小组织，适配对象池与组件存储；跨线程释放采用无锁单向队列回收至本地缓存。
- SmallObjectAllocator（小对象堆）：8–256 Bytes 的大小类分层，线程本地缓存 + 中央回收器；远端释放使用“批量转移”减少同步。
- SegregatedFits/TLSF：面向中等大小变长分配，保持近似 O(1) 的查找/释放复杂度，碎片率受控。
- PartitionAllocator（分区化）：同类对象/子系统独占分区，提升局部性与安全性；可选 Guard Pages。
- HugePageArena：在支持平台上采用大页（Windows Large Pages/2MB）以降低 TLB Miss；按需降级到普通页。
- SystemAllocator：基于 `Platform::Memory::Heap` 的兜底实现，供非性能关键路径/工具链使用。
- FallbackChain：组合多个策略（如 SmallObject → TLSF → System），在失败或不匹配时级联回退。
- TaggedAllocator：所有分配带 `MemoryTag`（Graphics/Audio/Physics/UI/Tooling…），用于观测与预算控制。

## 10. 并发与线程模型 (Concurrency Model)

- 线程本地缓存（TLC）：每个线程维护小对象缓存与最近使用块，减少跨线程竞争。
- 远端释放（Remote Free）：跨线程 `Deallocate` 不阻塞分配线程；通过无锁队列批量回收到拥有者线程。
- 统一序语：仅在必要边界使用细粒度原子操作；批处理设计减少 CAS 次数。
- 跨线程等价性：`IsEqual` 为真时允许跨实例释放；否则通过 `RemoteFree` 通道转交原拥有者。

## 11. 虚拟内存与大页 (VM & Large Pages)

- 两阶段提交：`Reserve` 与 `Commit` 分离，支持按需增长的连续 Arena；可设置 `Guard Pages` 检测越界。
- 延迟提交与预热：按使用模式（线性/随机）进行批量 `Commit` 或惰性提交；减少页错误。
- 大页支持：可选绑定到大页，提高带宽与降低 TLB Miss；回退策略在不可用时自动降级。

## 12. NUMA 感知 (NUMA Awareness)

- NUMA 节点绑定：Arena/Pool 可配置目标节点；小对象堆默认绑定到线程的本地节点。
- 读多写少场景：支持跨节点复制（只读结构）或分区化分配，降低远程访问延迟。

## 13. 可观测性与预算 (Observability & Budgets)

- 统一指标：分配/释放次数、失败码、峰值、活跃字节、碎片率、远端释放队列长度。
- 预算模型：按 `MemoryTag` 记录与限流；超限时返回特定 `StatusCode` 并触发告警事件。
- 采样与事件：与 Profiling 模块协作输出周期性快照与尖峰事件；支持低开销采样模式。

## 14. API 拓展 (API Extensions)

### 14.1 `IMemoryResource::Reallocate`

```cpp
virtual Expect<MemoryBlock> Reallocate(MemoryBlock block, USize newSize, USize align) noexcept;
```
- 语义：优先尝试原地扩容/缩容；失败时进行移动扩容并返回新 `MemoryBlock`。
- 合同：调用方必须提供旧块的精确 `size` 与 `align`；成功后旧块不再有效。

### 14.2 `OwnedMemoryBlock`

```cpp
struct OwnedMemoryBlock { /* 持有资源引用，RAII 释放 */ };
```
- 持有分配器引用，作用域结束自动 `Deallocate`；支持移动，不可复制。

### 14.3 `Allocator` 增强

- `ScopeMarker()`：为 Stack/Frame 分配器提供标记/回滚接口。
- `TryReallocate(ptr, newSize)`：对象级别的试探性重分配（必要时重建对象）。
- `GetInfo()`：查询资源名称、线程安全属性、大小类配置等元信息。

## 15. 使用指南 (Usage Guide)

- 暂存/高频：`FrameAllocator`，每帧一次 `Reset`。
- 递归/阶段：`StackAllocator` + `Marker`。
- 大量同尺寸对象：`PoolAllocator` 或 `SmallObjectAllocator`。
- 变长中等大小：`TLSF/SegregatedFits`。
- 大块连续空间：`HugePageArena` 或 `VirtualArena`。
- 工具/非关键：`SystemAllocator`。

## 16. 性能目标 (SLO)

- 小对象分配/释放：P50 < 80ns，P99 < 200ns（线程本地）。
- 远端释放合并：批量回收单次 < 2µs；稳态低抖动。
- 线性重置：百万级对象构建场景下 `Reset` < 150µs。
- 中等变长分配碎片率：< 1.5%（长期稳态）。
- 虚拟内存提交：批量提交吞吐与页错误率可控，支持预热配置。

## 17. 配置 (Configuration)

- 编译期：是否启用 Canaries/Leak-Detection、是否启用大页、默认对齐（如 16/32）。
- 运行时：`MemoryConfig`（大小类阈值、预算、NUMA 策略、采样频率）。

## 18. 示例与测试 (Samples & Tests)

- 样例：`Samples/SmallObjectBench`、`Samples/FrameAllocatorUsage`、`Samples/RemoteFreeDemo`、`Samples/ConcurrentPoolAllocatorTest`。
- **并发压力测试 (ConcurrentPoolAllocatorTest)**：
  - 多线程并发分配/释放测试，验证PoolAllocator的线程安全性和无锁实现
  - 采用Producer-Consumer模型，5秒持续压力测试
  - 验证内存完整性（写入0xDEADBEEF标记值）、原子操作正确性
  - 检测内存泄漏和统计信息一致性，确保分配器状态稳定
  - 支持线程亲和性和远端释放机制的端到端验证
- 基准：微基准（ns 级）与端到端场景（帧构建、资源加载）。
- 校验：越界检测、标签预算压力测试、跨线程释放一致性测试。

## 19. 安全性与调试 (Safety & Debug)

- Canary/Guard Pages、双向填充 Magic Numbers（Debug 构建）。
- 泄漏报告与调用栈（需 `StackTrace`）。
- 配额越界与异常路径降级（返回状态码，不抛异常）。

## 20. 术语表 (Glossary)

- Arena：线性增长的连续内存区域，支持批量重置。
- Size Class：按大小划分的小对象类别，用于快速查找与复用。
- Remote Free：跨线程释放通过队列或缓冲区回收到拥有者线程。
- Partition：逻辑分区，减少跨类别复用与提升局部性。