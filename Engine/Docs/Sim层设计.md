# Concinna Engine: Sim 层架构设计

## 1. 核心定位：数据的真理与时间的切片

在 Concinna 的架构中，`Sim` (Simulation) 层不仅仅是“游戏逻辑”，它是**确定性状态机**的物理载体。

*   **定位**：Sim 层是**无副作用**、**无 IO**、**内存紧凑**的数据集合。
*   **职责**：负责维护游戏世界在 $T$ 时刻的完整状态，并根据输入 $I$ 计算 $T+1$ 时刻的状态。
*   **禁忌**：严禁直接调用 `Sys.RHI`（渲染）、`Sys.Audio`（音频）、`Prm.File`（文件）或 `Prm.Time`（系统时间）。

## 2. 内存架构：基于快照的 ECS (Snapshot-able ECS)

为了适配 **Ouroboros Memory Model**（衔尾蛇内存模型），Sim 层的 ECS 实现必须彻底摒弃传统的堆分配模式（如 `std::vec` 或指针链接）。

### 2.1 状态块 (The State Block)
整个游戏世界在某一帧的状态必须被平铺在一个连续的内存块（`SimPage`）中。
*   **相对寻址**：内部所有引用必须使用 `EntityID` (Handle) 或 `RelPtr` (Offset)，禁止使用绝对指针。
*   **克隆成本**：状态的复制等同于 `memcpy`。

### 2.2 数据布局 (Archetype-SoA)
采用基于原型的 SoA 布局，但为了配合快照，需要特殊设计：
*   **Table 结构**：每个 Archetype 对应一个 Table。Table 不动态扩容，而是预分配在 `SimPage` 中（或使用基于页面的链表，但页必须在 Ring Buffer 内）。
*   **Component Storage**：组件数据以 SoA 形式紧密排列。
    *   *优势*：`Sys.Job` 可以利用 SIMD 批量处理组件（如 `Cap.Math` 中的 `vec3_fma_view`）。
    *   *IR 协同*：组件的内存布局由 `Sys.IR` 在编译期确定，生成对应的 Rust Struct 和 Offset Table。

## 3. 执行模型：确定性步进 (The Deterministic Step)

Sim 层的运行不再是“调用 Update”，而是“执行状态转移函数”。

### 3.1 阶段划分 (Phased Execution)
为了并行安全和确定性，一帧逻辑被严格切分为三个阶段：

1.  **Read Phase (并行)**：
    *   所有 System 并行启动。
    *   **只读**访问当前帧状态 $S_t$。
    *   **写入**临时命令缓冲区 (`FrameLinear` 内存)。例如：`Cmd::Move(Entity, Delta)` 或 `Cmd::Spawn(Prefab)`.
2.  **Resolve Phase (串行/分区并行)**：
    *   处理命令缓冲区。
    *   解决冲突（如两个 System 同时移动同一个 Entity，按确定性规则合并）。
    *   写入/构建下一帧状态 $S_{t+1}$。
3.  **Commit Phase (状态翻转)**：
    *   更新全局指针，将 $S_{t+1}$ 标记为最新状态。
    *   触发 `Sys.Scripting` 的协程恢复（如果脚本被挂起）。

### 3.2 脚本集成
*   **WASM**：直接将 ECS 的 Component 数组内存映射到 WASM 的线性内存中。WASM 看到的只是裸指针数组，零拷贝读写。
*   **Lua**：通过 Handle 访问。Lua 中持有 `EntityID`，调用 API 时，宿主在当前 `SimPage` 中查找数据。

## 4. 与 Sys.IR 的协同 (The Schema Definition)

`Sys.IR` 是 Sim 层启动的关键。我们不再手写 Component 结构体，而是定义 Schema。

### 4.1 定义流程
1.  **DSL/IR 定义**：
    ```text
    // 伪代码概念
    component Transform {
        position: vec3,
        rotation: quat,
    }
    ```
2.  **IR 编译 (Dev 阶段)**：
    *   `Sys.IR` 解析定义。
    *   生成 Rust 代码：`struct Transform { ... }`。
    *   生成 SoA 容器代码：`struct TransformStorage { x: Vec<f32>, ... }`。
    *   生成序列化/Hash 代码（用于校验确定性）。
3.  **运行时 (Sim 阶段)**：
    *   Sim 层直接使用生成的结构。

## 5. 与 RHI 的解耦：渲染代理 (Render Proxy)

这是解决“RHI 硬编码”问题的核心。Sim 层不知道 RHI，它只产生数据。

### 5.1 提取 (Extraction)
在 Sim 帧结束时，运行一个特殊的 System：`RenderExtractionSystem`。
*   **输入**：Sim 层的 `Transform`、`MeshID` (Asset Handle)、`MaterialParams`。
*   **输出**：**Render Packet**。
    *   这是一个存储在 `FrameLinear` 内存中的临时结构。
    *   包含：`InstanceData { mat4 model; u32 mesh_id; ... }` 的扁平数组。

### 5.2 消费 (Consumption)
`Eng` (Engine) 层持有 `Sys.RHI` 的 Device。
*   `Eng` 读取 **Render Packet**。
*   `Eng` 将 `AssetID` 映射为 RHI 的 `BufferHandle` / `TextureHandle`。
*   `Eng` 调用 `Sys.RHI` 录制 CommandBuffer。

**解耦点**：Sim 层只操作逻辑 ID（如 "Cube.mesh" 的 Hash），RHI 层只操作 GPU Handle。中间由 Engine 层维护映射表。

## 6. 总结：Sim 层的重构路线

为了启动 Sim 层，你需要按以下顺序实施：

1.  **Schema (IR 驱动)**：
    *   利用 `Sys.IR` 定义最基础的组件：`Transform`, `Tag`。
    *   编写过程宏或构建脚本，生成 Rust 的 SoA 结构。

2.  **State Container (内存)**：
    *   实现 `SimWorld`，它持有一个 `Cap.Memory` 的分配器引用。
    *   实现 `Snapshot` 机制：`SimWorld` -> `Blob`。

3.  **System Scheduler (Sys.Job)**：
    *   定义 `System` trait。
    *   实现一个简单的调度器，按依赖关系执行 System。

4.  **Render Extract (连接 RHI)**：
    *   定义 `RenderPacket` 结构。
    *   实现从 `SimWorld` 拷贝数据到 `RenderPacket` 的逻辑。

通过这种设计，Sim 层保持了纯粹的数据处理特性，既满足了确定性需求，又通过中间数据结构完美解耦了渲染层。