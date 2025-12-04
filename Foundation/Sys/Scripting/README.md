# Concinna Engine - Sys.Scripting 模块设计文档

**版本：** 0.1.0 (Draft)
**状态：** 规划中
**依赖：** `Sys.Job`, `Sys.VFS`, `Cap.Memory`, `Cap.Reflection`

---

## 1. 概述 (Overview)

`Sys.Scripting` 是 Concinna 引擎的统一脚本运行时环境。它旨在解决游戏开发中“高性能计算”与“快速逻辑迭代”之间的矛盾。本模块不负责脚本的编译（这是 `Dev` 层的职责），只负责字节码的加载、沙盒化执行以及与引擎核心数据的交互。

### 1.1 设计哲学
*   **双层架构：** 采用 **WASM** 处理计算密集型逻辑（Sim 状态更新），采用 **Lua** 处理高层胶水逻辑（UI、关卡流程）。
*   **引擎主导：** 用户提供源码，引擎提供运行时。引擎拥有对内存、I/O 和 CPU 时间片的绝对控制权。
*   **确定性优先：** 运行时环境必须是确定性的，以支持回滚网络代码（Rollback Networking）和重播调试。
*   **纤程亲和：** 脚本执行必须能够被 `Sys.Job` 调度器挂起和恢复，绝不允许阻塞 OS 线程。

---

## 2. 技术选型 (Technical Stack)

鉴于 Concinna 拒绝标准库重磅功能的原则，我们选择生态中可嵌入、低依赖且高性能的组件。

### 2.1 WASM 运行时 (Performance Core)
*   **选型：** **`wasmtime`** (Cranelift 后端)
*   **理由：**
    *   Rust 原生，内存安全。
    *   支持 JIT 编译，运行速度接近原生。
    *   提供精细的 Host Function 定义能力，易于集成 `Sys.VFS`。
    *   支持 Fuel 机制（指令计数），可强制中断死循环脚本。

### 2.2 Lua 运行时 (Iteration Core)
*   **选型：** **`mlua`** + **`LuaJIT`** (静态链接)
*   **理由：**
    *   `mlua` 提供了安全的 Rust 绑定层。
    *   `LuaJIT` 是目前最快的动态语言解释器之一，且 FFI 性能极佳。
    *   支持协程（Coroutine），易于映射到引擎的 Fiber 系统。

### 2.3 接口绑定 (Binding)
*   **WASM:** 使用 **`wit-bindgen`** 定义 Host-Guest 接口标准，自动生成 Rust FFI 代码。
*   **Lua:** 使用 `mlua` 的 `UserData` trait 暴露引擎对象。

---

## 3. 架构设计 (Architecture)

`Sys.Scripting` 对外提供统一的 `ScriptRuntime` 接口，内部维护两种 VM 的实例池。

### 3.1 运行时隔离与沙盒 (Sandbox)

为了保证确定性和安全性，必须切断脚本对 OS 的直接访问。

| 功能 | 原生行为 | Concinna 策略 |
| :--- | :--- | :--- |
| **文件 I/O** | `std::fs`, `io.open` | **拦截**。重定向至 `Sys.VFS`，仅允许读取 `project:` 挂载点下的资产。 |
| **网络** | `std::net`, `socket` | **禁用**。仅允许通过 `Sys.Network` 提供的 Host Function 发送特定消息。 |
| **时间** | `std::time`, `os.time` | **替换**。提供 `Engine.get_gametime()`，返回确定性的逻辑帧时间。 |
| **随机数** | `rand`, `math.random` | **替换**。提供基于种子确定性的 `Cap.Random` 接口。 |
| **打印** | `println!`, `print` | **重定向**。输出到 `Cap.Log` 或编辑器控制台。 |

### 3.2 纤程调度集成 (Fiber Integration)

这是本模块最核心的工程挑战。脚本的执行不能阻塞物理线程。

*   **Lua 方案：**
    *   所有 Lua 脚本在 `coroutine` 中运行。
    *   当脚本调用耗时操作（如 `Wait(1.0)`）时，Lua 协程 `yield`。
    *   Rust Host 端捕获 `yield`，调用 `Sys.Job::suspend_current()` 挂起当前 Fiber。
    *   定时器触发后，`Sys.Job` 恢复 Fiber，Rust Host 再次 `resume` Lua 协程。

*   **WASM 方案：**
    *   利用 `wasmtime` 的 **Async Support** (基于 Stack Switching)。
    *   当 WASM 调用 Host Function `sys_yield()` 时，Host 端挂起 Future/Fiber。
    *   这要求 WASM 模块必须以“可重入”的方式编写或编译。

### 3.3 数据交互桥接 (Data Bridge)

避免在 Host 和 Guest 之间进行大量的数据拷贝（Marshal/Unmarshal）。

*   **WASM (Shared Memory / Linear Memory):**
    *   引擎将 `Sim.Component` 的 SoA 数据块直接映射或拷贝到 WASM 实例的线性内存中。
    *   WASM 代码通过指针直接操作这些数组（零拷贝或单次批量拷贝）。
    *   **关键技术：** `Cap.Memory` 分配器需对齐 WASM 页大小 (64KB)。

*   **Lua (Light UserData / FFI):**
    *   通过 LuaJIT FFI 暴露 C-ABI 的结构体指针。
    *   Lua 直接读写内存地址，避免通过 Lua Stack 传递大量浮点数。

---

## 4. 模块接口定义 (API Surface)

`Sys.Scripting` 应该暴露极简的接口给 `Eng` 层。

```rust
// 伪代码设计

pub enum ScriptType { Lua, Wasm }

pub struct ScriptModule {
    // 编译后的字节码或模块句柄
    internal: ModuleHandle 
}

pub struct ScriptInstance {
    // 运行时的上下文（Lua State 或 Wasm Store）
    context: ContextHandle,
    // 脚本内部状态的内存
    memory: MemoryHandle
}

impl ScriptSystem {
    // 初始化运行时环境（注册 Host Functions）
    pub fn init(vfs: &Vfs) -> Self;

    // 加载脚本代码（从 VFS 读取 -> 编译/验证 -> 模块）
    pub fn load_module(&self, path: &str) -> Result<ScriptModule>;

    // 实例化脚本（分配内存，准备执行环境）
    pub fn instantiate(&self, module: &ScriptModule) -> Result<ScriptInstance>;

    // 执行帧更新（传入当前帧的 Sim 数据视图）
    // 此函数内部会处理 Fiber 的挂起和恢复
    pub fn tick(&self, instance: &mut ScriptInstance, data: &SimDataView);
}
```

---

## 5. 实现路线图 (Implementation Roadmap)

### 阶段一：基础集成 (Foundation)
1.  **引入依赖：** 在 `Cargo.toml` 中添加 `wasmtime` 和 `mlua`。
2.  **Hello World：** 实现 `run_lua_string` 和 `run_wasm_bytes`，验证基本的 Host Function 调用（如 `print`）。
3.  **VFS 集成：** 实现 `Lua` 的 `require` 加载器和 `WASM` 的 `wasi` 文件接口，使其通过 `Sys.VFS` 读取文件。

### 阶段二：沙盒与安全 (Security)
1.  **Lua 沙盒：** 编写初始化脚本，清空 Lua 全局环境表 `_G`，只注入允许的 API。
2.  **WASM 限制：** 配置 `wasmtime` 的 `Config`，限制最大内存使用量和 CPU 指令消耗。

### 阶段三：数据互操作 (Interop)
1.  **WASM 内存映射：** 实现 Host 向 WASM 线性内存写入 `Transform` 数组的逻辑。
2.  **Lua FFI：** 使用 `mlua` 暴露 `Sim` 层组件的 C 结构体布局。

### 阶段四：调度器融合 (Scheduling)
1.  **Fiber 桥接：** 实现 `Sys.Job` 与脚本挂起机制的对接。
2.  **热重载 (Hot Reload)：** 配合 `Sys.HotReload`，实现运行时无缝替换 `ScriptModule` 并尝试保留 `ScriptInstance` 状态（高级特性）。

---

## 6. 总结

`Sys.Scripting` 不是一个简单的脚本解释器，它是 Concinna 引擎 **“数据驱动”** 和 **“逻辑分离”** 架构的关键枢纽。通过严格区分 WASM（高性能计算）和 Lua（高层逻辑），并统一在 Fiber 调度器下管理，我们能够兼顾 3A 级引擎的性能要求和独立游戏开发的灵活性。