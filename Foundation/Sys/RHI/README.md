### 1. 核心原则

1.  **所有权与借用 (Ownership & Borrowing)**：
    *   利用 Rust 的生命周期（Lifetimes）确保 `CommandList` 录制期间引用的资源不会被释放。
    *   利用 `&mut self` 保证单线程录制的独占性，利用 `Send/Sync` 标记保证多线程分发的安全性。

2.  **类型安全句柄 (Type-Safe Handles)**：
    *   使用 **NewType Pattern** (如 `struct BufferHandle(u32)`) 代替 `StrongAlias`。
    *   配合 **Generational Arena**（代际索引池）来管理资源，彻底解决 GPU 资源的 "Use-After-Free" 问题（即 CPU 释放了句柄，但 GPU 还在用）。

3.  **枚举分发 (Enum Dispatch) vs 静态分发**：
    *   **C++ 痛点**：虚函数开销。
    *   **Rust 方案**：
        *   **方案 A (极致性能)**：使用 `#[cfg(feature = "vulkan")]` 进行编译期静态分发。编译出的二进制只包含一种后端，零运行时开销。
        *   **方案 B (灵活性)**：使用 `enum Backend { Vulkan(VkBackend), DX12(Dx12Backend) }`。Rust 的 match 优化通常比 C++ 虚表更高效（且利于内联）。
        *   **建议**：鉴于 Concinna 追求“最高控制力”，推荐 **方案 A**。对外暴露统一的 `struct Device`，内部实现根据编译选项切换。

4.  **类型状态模式 (Typestate Pattern)**：
    *   利用 Rust 的泛型状态机来防止 API 误用。例如，`CommandBuffer` 可以有 `Recording` 和 `Executable` 两种状态，只有 `Recording` 状态才能调用 `draw`，只有 `Executable` 状态才能提交给队列。这在编译期就能捕获逻辑错误。

---

### 2. 模块结构 (`Sys.RHI`)

```text
Sys.RHI/
├── API/
│   ├── Handle.rs          # 强类型句柄 (BufferHandle, TextureHandle)
│   ├── Device.rs          # 设备抽象，资源创建入口
│   ├── Command.rs         # CommandBuffer 录制接口
│   ├── Pipeline.rs        # PSO, BindGroupLayout
│   ├── Descriptor.rs      # BindGroup (资源绑定集合)
│   └── Sync.rs            # Barrier, Fence, Semaphore
├── Backend/
│   ├── Vulkan/            # 基于 ash 的实现
│   └── DX12/              # 基于 windows-rs 的实现
├── Allocator/             # 显式内存管理 (集成 gpu-allocator)
└── lib.rs                 # 统一导出
```

---

### 3. 关键技术选型

1.  **Vulkan 绑定**: **`ash`**。
    *   它是 Rust 生态中最底层的 Vulkan 绑定，不做任何隐藏逻辑，完全符合“最高控制力”哲学。
2.  **DX12 绑定**: **`windows-rs`**。
    *   微软官方维护，直接映射 COM 接口，零开销。
3.  **内存分配器**: **`gpu-allocator`**。
    *   这是一个纯 Rust 编写的库，功能对标 VMA/D3D12MA。
    *   **优势**：无需 C++ FFI 开销，内存布局完全在 Rust 控制之下，且支持 Vulkan 和 DX12。
4.  **Shader 编译**: **`hassle-rs` (DXC bindings)**。
    *   直接调用 DXC 动态库。符合你“统一使用 HLSL”的策略。

---

### 4. 核心组件设计

#### 4.1 句柄系统 (The Handle System)

不要传递指针或 `Arc<T>` 给 RHI 用户。

```rust
// 使用宏生成强类型句柄，零成本
macro_rules! define_handle {
    ($name:ident) => {
        #[derive(Clone, Copy, PartialEq, Eq, Hash, Debug)]
        pub struct $name {
            pub(crate) index: u32,
            pub(crate) generation: u32,
        }
    };
}

define_handle!(BufferHandle);
define_handle!(TextureHandle);
define_handle!(PipelineHandle);
```

#### 4.2 资源管理与内存 (Resource & Memory)

将资源创建与内存分配解耦，但提供辅助函数。

*   **Transient (瞬态)**：使用线性分配器（Linear Allocator）或环形缓冲（Ring Buffer），每帧重置。
*   **Persistent (持久)**：使用 `gpu-allocator` 进行块管理。

**Rust 特性应用**：使用 `Option<NonNull<T>>` 优化空指针检查，利用 `Drop` trait 自动回收 CPU 端元数据（但 GPU 资源释放需显式调用，因为涉及帧同步）。

#### 4.3 绑定模型 (BindGroup)

采用 WebGPU 风格的 `BindGroup` 设计，这是对 Vulkan DescriptorSet 和 DX12 DescriptorTable 的最佳抽象。

1.  **`BindGroupLayout`**: 定义“插槽”的形状（类型、阶段）。
2.  **`BindGroup`**: 实际的资源集合。

**策略**：
*   在 Vulkan 后端，维护一个全局的 `DescriptorPool` 管理器。
*   实现 **BindGroup Cache**：如果用户请求创建相同的 BindGroup（资源句柄相同），直接返回缓存的 Handle。这利用了 Rust 的 `HashMap` 和 `Hash` trait。

#### 4.4 命令列表 (CommandList)

这是 RHI 的核心。

```rust
pub struct CommandBuffer<'a> {
    // 引用后端具体的 CmdBuf，生命周期 'a 确保 CmdBuf 不会活得比 Device 久
    backend: &'a mut BackendCommandBuffer, 
}

impl<'a> CommandBuffer<'a> {
    // 1. 状态设置 (无状态设计，直接透传)
    pub fn set_pipeline(&mut self, pipeline: PipelineHandle) { ... }
    
    // 2. 资源绑定 (BindGroup 模型)
    // set_index 对应 Vulkan set=N
    pub fn set_bind_group(&mut self, set_index: u32, group: BindGroupHandle, dynamic_offsets: &[u32]) { ... }

    // 3. 绘制
    pub fn draw(&mut self, vertex_count: u32, instance_count: u32, ...) { ... }

    // 4. 同步 (显式 Barrier)
    // 利用 Rust 的 Slice 传递批量 Barrier
    pub fn resource_barrier(&mut self, barriers: &[ResourceBarrier]) { ... }
}
```

---

### 5. 针对 Concinna 的 Rust 特性升级

#### 5.1 编译期状态机 (Typestate for CommandBuffers)

防止在 `End()` 之后调用 `Draw()`，或者在 `RenderPass` 之外调用 `Draw`。

```rust
pub struct CmdBuf<State> { ... }
pub struct Recording;
pub struct Executable;
pub struct InsideRenderPass;

impl CmdBuf<Recording> {
    pub fn begin_render_pass(self, ...) -> CmdBuf<InsideRenderPass> { ... }
    pub fn finish(self) -> CmdBuf<Executable> { ... }
}

impl CmdBuf<InsideRenderPass> {
    pub fn draw(&mut self, ...) { ... }
    pub fn end_render_pass(self) -> CmdBuf<Recording> { ... }
}
```
*注：这会增加 API 复杂度，初期可以先用运行时断言（`debug_assert!`），后期再升级为 Typestate。*

#### 5.2 线程安全录制 (Send + Sync)

利用 `Sys.Job` 系统。
*   `CommandPool` 是线程局部的（Thread-Local）。
*   `CommandBuffer` 是 `!Sync` 的（只能在一个线程录制），但是 `Send` 的（录制完可以发送给主线程提交）。
*   Rust 的编译器会自动检查这些约束，防止多线程竞争 CommandBuffer。

#### 5.3 宏与 DSL (Shader Generation)

利用 Rust 强大的宏系统，你可以为 `Sys.IR` 编写过程宏（Proc Macros）。
*   用户在 Rust 代码中定义 Shader Graph 结构。
*   宏在编译期将其转换为 `Sys.IR` 的中间表示。
*   `Dev.AssetCompiler` 读取这些 IR 并调用 DXC。

---

### 6. 封装与开发顺序 (Roadmap)

建议的开发步骤，优先打通流程：

1.  **基础层 (The Base)**:
    *   集成 `ash` 和 `gpu-allocator`。
    *   实现 `Device` 创建和 `Queue` 获取。
    *   实现 `SwapChain` (对接 `Prm.WSI`)。

2.  **资源层 (The Resources)**:
    *   实现 `Buffer` 和 `Texture` 的创建/销毁。
    *   实现 **Staging Buffer** 机制（用于 CPU->GPU 数据上传）。
    *   实现 `GenerationalArena` 来管理 Handle。

3.  **管线层 (The Pipeline)**:
    *   集成 `hassle-rs` (DXC)。
    *   实现 `ShaderModule` 加载。
    *   实现 `GraphicsPipeline` 创建（硬编码一些状态，后续再暴露全部配置）。

4.  **命令层 (The Commands)**:
    *   实现 `CommandPool` 管理。
    *   实现 `CommandBuffer` 录制接口（Begin, End, Draw）。
    *   **关键点**：先实现最简单的 Barrier，不要试图自动推导。

5.  **绑定层 (The Binding - Hardest Part)**:
    *   实现 `BindGroupLayout` 和 `BindGroup`。
    *   在 Vulkan 后端处理 `DescriptorSet` 的分配和更新。

6.  **多线程验证**:
    *   结合 `Sys.Job`，测试多线程录制 CommandBuffer 并提交。

### 7. 总结

Rust 版本的 RHI 应该比 C++ 版本更**安全**且**同样高效**。

*   **宏观架构**：`Frontend (Handles/Enums)` -> `Backend (Ash/Windows-rs)`。
*   **关键差异**：使用 `gpu-allocator` 替代 C++ 分配器；使用 `Enum Dispatch` 或 `cfg` 替代虚函数；使用 `BindGroup` 统一资源绑定模型。
*   **核心优势**：利用 Rust 的生命周期强制执行资源同步规则，利用 `Send/Sync` 保证多线程录制的安全性。