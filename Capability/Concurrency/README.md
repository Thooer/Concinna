# **Nova Engine Concurrency V3.1 设计**

---

# **1. 设计目标 (Design Goals)**

Concurrency 3.1 不仅仅是一个 N:M 调度器，而是一个**软硬协同、计算与IO协同、统一无锁抽象与可观测性的异构运行时**。

核心目标：在维持 C++/手写汇编级别的极限性能（**<20ns 切换**）的同时，让整个系统具备最高等级的：
*   **稳定性 (Stability)**
*   **可调试性 (Debuggability)**
*   **无锁算法一致性 (Consistency)**
*   **第三方生态兼容性 (Compatibility)**
*   **可维护性 (Maintainability)**

本阶段的进化方向：从“功能正确”提升到 **“微架构友好 + 全链路零阻塞 + 半通用无锁抽象”**。

---

# **2. 架构四大支柱 (The Four Pillars)**

## **支柱一：微架构亲和**

### **1. 拓扑感知调度域**
*   **拓扑构建**：启动时基于 CPUID 构造 `Socket > CCX > Core > SMT` 拓扑树。
*   **亲和性调度**：Worker 的任务选择顺序遵循**三层亲和**原则：
    1.  本地栈（L1/L2 Cache 命中）
    2.  同 CCX/Socket（共享 L2/L3）
    3.  远端窃取（带退避惩罚）
*   **效果**：大幅降低跨 NUMA 流量。

### **2. 缓存行防御体系**
*   **Stack Coloring**：Fiber 栈采用随机着色，降低 L1 Set Collision。
*   **结构体隔离**：Worker 结构体划分为 `Read-Mostly` 区与 `Read-Write` 区，分别进行 `alignas(128)` 隔离。
*   **伪共享防御**：Deque 的热点成员（top/bottom）强制拆分，避免 MESI 协议抖动（Thrash）。

### **3. 指令流优化**
*   **手写汇编**：针对 x64/ARM64 手写汇编实现最小化上下文切换。
*   **寄存器策略**：默认仅保存 Callee-Saved GPR；SIMD 操作走 HeavyJob 专用分支。
*   **预取优化**：跳转前预取目标栈页，降低切换抖动。
*   **安全兼容**：兼容 CET / Shadow Stack。

### **4. 系统调用优化**
*   **滞后唤醒**：若有自旋 Worker，则暂不调用 futex 唤醒。
*   **批处理提交**：支持多个任务一次性唤醒，减少 syscall 开销。

---

## **支柱二：可插拔 IO 驱动 (Pluggable IO Driver)**

### **1. 标准驱动协议 (IDriver)**
*   **核心理念**：调度器仅管理调度，不直接触碰 fd/socket。
*   **接口约束**：驱动必须实现 `PollNow()` 和 `Park(timeout)`。
*   **控制反转**：实现 IO Loop 与调度 Loop 的**完全控制反转**。

### **2. 原生定时器集成**
*   **无滴答设计**：无独立 Tick 线程，Worker 本身即时钟。
*   **最小堆管理**：调用 Park 前检查最小堆顶定时器。
*   **原子缓存**：使用 `next_expiry` 原子变量缓存，减少锁竞争。

### **3. 混合调度循环**
新的 Worker 主循环逻辑演进为：
> **执行 (Execute) → 窃取 (Steal) → PollNow(IO) → Park(futex)**

*IO 事件填充 CPU 计算间隙，实现计算与 IO 的完美共生。*

### **4. 生态兼容性**
提供适配层，可无缝接入 **gRPC**、**SteamSockets**、**Asio** 等主流库。

---

## **支柱三：防御性调试体系 (Defensive Debugging)**

### **1. 双模切换**
*   **Debug 模式**：走 OS Fiber，IDE 可见完整调用栈。
*   **Release 模式**：走手写汇编，极致性能模式。

### **2. ASan 适配 & 栈金丝雀**
*   **ASan Hooks**：切换前后调用 ASan fiber hooks，精准捕捉栈越界。
*   **Stack Canary**：在每个 Fiber 栈底埋设 Canary 值，调度器空闲时抽查。

### **3. 全链路可观测性**
*   **Tracy 集成**：切换点、锁等待点、IO 轮询点全埋点。
*   **验收标准**：满载时 Worker 时间轴在 Profiler 中应呈**“实心砖块”**状。
*   **死锁看门狗**：实时记录 Fiber 的持有/等待关系图。

---

## **支柱四：开发者体验 (Developer Ergonomics)**

### **1. QoS 优先级**
Worker 内部维护两个 Deque（High/Normal），窃取逻辑优先处理高优先级任务。

### **2. Awaitable 胶水层**
将 IO、定时器、NextFrame 统一封装为 **Awaitable** 对象，差异对用户透明。

### **3. 结构化并发**
支持异常自动传播、任务级联取消。

### **4. 内存系统深度融合**
每个 Worker 绑定 FrameAllocator，Job 结束时自动回滚内存，实现零碎片。

---

# **3. Lock-Free 策略化抽象 (核心新增)**

Concurrency 3.0 的无锁设计采用**半通用抽象 (Semi-General Abstraction)**。
我们抽象的是 **结构模式 + 回收策略 + 并发角色**，而非具体的容器语义。这是满足“极致性能”与“大规模可维护性”的唯一平衡点。

## **3.1 为什么需要半通用？**

引擎需要大量的无锁结构（如 WS-Deque, Actor Mailbox, Fiber Ready Queue 等）。
如果每个队列都独立手写，会导致：
*   无锁协议碎片化
*   难以维护
*   Padding/Memory Order 容易出错

但如果企图做“完全通用容器抽象”，会导致：
*   ABA 问题激增
*   Atomic 冲突加剧
*   性能崩溃

因此，我们采用**半通用**方案：
> **“抽象队列模型，不抽象队列行为。”**

## **3.2 策略化无锁结构工厂 (LockFreeFactory)**

Concurrency 3.0 新增统一的无锁结构工厂，通过模板组合生成具体结构：

```cpp
// 示例：通过工厂生成不同类型的无锁队列
MakeQueue<SPSC, Bounded, Epoch>();
MakeQueue<MPSC, Bounded, Hazard>();
MakeQueue<MPMC, Unbounded, Epoch>();
MakeWorkDeque<OwnerSteal, Epoch>();
```

### **可组合策略库**

| 策略类型 | 选项 |
| :--- | :--- |
| **结构策略** | SPSC, MPSC, MPMC, Work-Stealing Deque |
| **容量策略** | Bounded (预分配), Unbounded (分段链表/块池) |
| **回收策略** | Epoch, Hazard Pointer, Reclaimer(None) [Fiber专用] |
| **等待策略** | Spin, Backoff, Park (for IO driver) |

**优势：**
*   所有 Lock-Free 行为保持一致。
*   所有回收策略集中管理。
*   自动保证 Padding/Memory Order 正确性。
*   性能可随策略切换而平滑演进。

## **3.3 WS-Deque 与策略系统的结合**

Work-Stealing Deque 是调度器的生命线，我们以策略模式构造：

```cpp
using WorkerDeque = MakeWorkDeque<
    OwnerSteal,              // 角色语义
    Epoch,                   // 回收策略
    TopBotSplit<128,128>     // 热点隔离
>;
```

**收益：**
*   保证 **Owner** 的 push/pop 无锁且无共享。
*   确保 **Steal** 的 CAS path 逻辑正确。
*   全局统一 Hotspot Padding 布局。
*   一处改动，全局所有 Deque 即刻获益。

## **3.4 半通用抽象的最终作用**
1.  **性能完全保留**（不引入冗余语义）。
2.  **无锁协议集中化**（大幅减少并发错误）。
3.  **未来可扩展**（更换回收策略无需修改业务代码）。
4.  **更易调试**（统一结构，统一诊断工具）。
5.  **深度融合**（与调度器/IO 驱动共享生命周期）。

---

# **4. 核心数据结构 (整合升级)**

## **4.1 Chase-Lev Work-Stealing Deque**
基于策略系统生成：
*   本地 LIFO，窃取 FIFO。
*   无锁 CAS 保护 Top 指针。
*   采用 `TopBotSplit` 热点隔离策略。
*   自由选择 Epoch/Hazard 回收机制。

## **4.2 MPSC / MPMC Queue**
采用 LockFreeFactory 生成：
*   固定/可变容量可选。
*   Memory Order 可配置。
*   Padding 自动应用。
*   支持 **Bulk Push**（降低屏障总成本）。

## **4.3 Counter-Based 依赖图**
用于结构化并发的核心：
*   原子 Decrement 操作。
*   Zero-hit 触发 Continuation。
*   可选回收策略：`Reclaimer(None)`。

## **4.4 Parker (定向唤醒机制)**

 每个 Fiber 维护独立的唤醒信号量。

* 每个 Worker 拥有独立的 Parker(基于 `BinarySemaphore` 或 `AutoResetEvent`)。
* 惊群效应 -> 滞后唤醒：任务提交时仅唤醒一个空闲 Worker(调度器维护 `IdleMask` 或 `IdleStack`)。
* 支持 IO 唤醒特定 Worker。

---

# **5. 性能指标 (保持不变)**

*   **上下文切换**：< 20ns
*   **调度 Overhead**：< 5%
*   **IO 延迟增加**：< 1us
*   **Ping-Pong 测试**：< 200ns
*   **Debug 模式**：栈完全可见
*   **Release 模式**：Tracy 全链路可观测

---

# **6. 验证与测试 (Verification)**

### **6.1 极限压力测试 (Torture Tests)**
*   **Tree Explosion**：单任务递归生成 100 万子任务，验证 Allocator 耗尽边界与 Deque 溢出回退机制。
*   **False Sharing Detector**：特制测试用例，所有 Worker 疯狂读写相邻原子变量，通过 `perf c2c` 验证缓存行隔离有效性。
*   **内存泄漏检测**：使用 Valgrind/ASan 验证零运行时分配和零泄漏。

### **6.2 性能基准测试**
*   **吞吐基准**：对比 Go Runtime, Tokio, Naughty Dog Job System。
*   **延迟基准**：测量不同负载下的尾延迟 (Tail Latency)。
*   **缓存性能**：使用 `perf stat` 验证缓存命中率提升。

### **6.3 Lock-Free 协议一致性测试**
*   策略组合交叉测试。
*   伪共享探测 (`perf c2c`)。
*   内存回收压力测试。
*   ABA 模拟测试。

---

# **7. 演进路线图 (Roadmap)**

### **第一阶段：稳固地基 (Hardening)**
*   **重点**：调试与安全。
*   **行动**：
    1.  实现 Windows Fiber API 的 Debug 适配。
    2.  集成 Tracy Profiler。
    3.  实现栈金丝雀检测。
    4.  实现基础的手写汇编上下文切换。
*   **产出**：一个 **"修得好"** 的调度器。

### **第二阶段：连接世界 (Connectivity)**
*   **重点**：IO 融合。
*   **行动**：
    1.  定义 `IDriver` 接口。
    2.  改造 Worker 循环，加入 `Poll/Park` 逻辑。
    3.  实现原生定时器集成（无滴答调度）。
    4.  实现基于 `eventfd` 的最小化驱动 Demo。
*   **产出**：一个 **"能上网"** 的调度器。

### **第三阶段：极致性能 (Optimization)**
*   **重点**：微架构优化。
*   **行动**：
    1.  实现 CPU 拓扑探测与分层窃取算法。
    2.  实施栈着色与结构体布局优化。
    3.  编写汇编级 ASan 钩子。
    4.  实现系统调用批处理优化。
    5.  完善优先级 QoS 系统。
    6.  **WS-Deque 策略化改造。**
    7.  **MPSC/MPMC 统一回收策略。**
*   **产出**：**高性能**运行时和 LockFreeFactory。

### **第四阶段：生态融合 (Ecosystem)**
*   **重点**：第三方库兼容。
*   **行动**：
    1.  完成 Awaitable 胶水层实现。
    2.  提供主流网络库的驱动适配（gRPC, SteamSockets, Asio）。
    3.  完善结构化并发增强。
    4.  优化内存子系统（Frame Allocator 集成）。
    5.  **策略化队列正式 API。**
    6.  **Fiber/Awaitable 与无锁结构深度融合。**
*   **产出**：一个 **"生态友好"** 的运行时。

---

# **8. 结语**

> **CPU → 调度 → IO → 无锁结构 → 内存系统 → 调试。**

**Concurrency 3.1** 使得 “高性能框架”->“全栈异构运行时”