### 1. 核心数据模型 (The Data Model)

这是 IR 的物理存储形式。为了通用，它必须是“无类型”的（Type-Erased），所有具体语义由元数据描述。

#### 1.1. 基础元素
*   **Context (上下文)**: 全局单例（或线程局部），拥有所有的类型存储、字符串池和内存分配器（Arena Allocator）。
*   **Operation (Op, 指令)**: 图中的节点。这是最小的执行单元。
    *   **结构**:
        *   `OpName`: 字符串标识符 (e.g., "math.add").
        *   `Operands`: 输入值列表 (`std::vector<Value*>`).
        *   `Results`: 输出值列表 (`std::vector<Value*>`).
        *   `Attributes`: 编译期常量字典 (e.g., `{ "unroll_factor": 4 }`).
        *   `Regions`: 嵌套的代码块（用于控制流，如 if/for 的函数体）。
        *   `Successors`: 跳转目标（用于分支指令）。
*   **Value (值)**: 图中的边。连接两个 Op。
    *   **SSA 属性**: 每个 Value 只能被定义一次 (Def)，但可以被使用多次 (Use)。
    *   **Use-Def Chain**: 维护链表，让 Value 知道谁使用了它（用于快速遍历和死代码消除）。
*   **Type (类型)**: 描述 Value 的元数据。
    *   IRSystem 只提供 `Type` 基类。
    *   具体类型如 `Math::Float32Type`, `RHI::TextureType` 由各方言派生。
*   **Block (基本块)**: 包含一个指令列表 (`List<Operation>`)。块内指令顺序执行。
*   **Region (区域)**: 包含一个控制流图（CFG），即多个 Block 的集合。

#### 1.2. 内存布局 (关键优化)
为了避免指针满天飞导致的 Cache Miss，采用 **基于区域的内存管理 (Region-based Memory Management)**。
*   所有的 Op, Value, Type 都分配在 `Context` 拥有的 `BumpPtrAllocator` 上。
*   销毁时直接释放整个 Context，不进行单个对象的 delete。

---

### 2. 方言系统 (The Dialect System)

这是实现“通用性”的关键。IRSystem 本身是空的，它通过加载“方言”来获得能力。

#### 2.1. Dialect (方言基类)
*   每个模块（Math, RHI）必须实现一个 `Dialect` 子类。
*   **职责**:
    *   注册该方言下的所有 Op (e.g., `MathDialect` 注册 `AddOp`, `MulOp`).
    *   注册该方言下的所有 Type.
    *   提供解析（Parsing）和打印（Printing）钩子，用于调试显示。

#### 2.2. Operation Definition (Op 定义)
不要手写 C++ 类来定义 Op。使用 **CRTP (Curiously Recurring Template Pattern)** 和 **Traits** 来注入行为。

```cpp
// 示例：Math 模块定义 AddOp
// 继承自 Op 基类，并注入 "NoSideEffect" (纯函数) 特性
class AddOp : public IR::Op<AddOp, IR::OpTrait::NoSideEffect> {
public:
    // 唯一的标识符
    static llvm::StringRef getOperationName() { return "math.add"; }

    // 方便的 C++ 访问器 (封装底层的 generic operands)
    Value* getLHS() { return getOperand(0); }
    Value* getRHS() { return getOperand(1); }
};
```

---

### 3. 变换与优化引擎 (The Transformation Engine)

这是你担心的“NP-Hard”问题的解决方案。我们不进行全图同构搜索，只进行**基于模式的贪婪重写**。

#### 3.1. Pattern Rewriter (模式重写器)
*   **工作原理**: 遍历图中的每一个 Op。对于每个 Op，询问已注册的 Patterns：“你是这个 Pattern 的根节点吗？”
*   **复杂度**: $O(N \times K)$，其中 N 是指令数，K 是 Pattern 数。这是线性的，非常快。

#### 3.2. Pattern 的结构 (C++ 接口)
```cpp
class RewritePattern {
public:
    // 1. 匹配阶段 (Match)
    // 检查 op 是否符合该模式的根节点特征
    virtual LogicalResult match(Operation* op) const = 0;

    // 2. 重写阶段 (Rewrite)
    // 修改图结构：创建新 Op，替换旧 Op 的结果
    virtual void rewrite(Operation* op, PatternRewriter& rewriter) const = 0;
};
```

#### 3.3. Pass Manager (Pass 管理器)
*   管理一系列 Pass 的执行顺序。
*   **Canonicalizer Pass**: 这是一个内置的特殊 Pass，它不断地应用所有方言注册的“规范化 Pattern”（如 $x+0 \to x$），直到图收敛（不再变化）。

---

### 4. 统一架构的工作流 (The Workflow)

让我们看看 Math, Algorithm, RHI 如何在这个系统中协作。

#### 阶段 1: 构建 (Builder) - C++ 前端
用户调用 Math 模块提供的 Builder API。此时不进行计算，只生成 IR 节点。

```cpp
// 用户代码
auto val = builder.create<Math::AddOp>(loc, a, b);
```
*   **IR 状态**: 图中增加了一个 `math.add` 节点。

#### 阶段 2: 高阶优化 (High-Level Optimization) - Algorithm 介入
运行 `Algorithm` 提供的 Pass。
*   **输入**: 包含大量 `math` 指令的 IR。
*   **动作**: 识别特定的数学结构（例如：连续的矩阵乘法），将其替换为更高效的算法节点，或者进行代数化简。
*   **IR 状态**: `math.mul` 序列可能变成了 `algo.gemm` (通用矩阵乘) 节点。

#### 阶段 3: 降级 (Lowering) - 决定去向
这是分叉点。根据目标（AOT, JIT, GPU），选择不同的 Lowering Pass。

*   **路径 A: CPU 执行 (JIT/AOT)**
    1.  **Lowering**: 将 `math.add` 转换为 `llvm.add`，将 `algo.gemm` 展开为嵌套循环 (`scf.for`)。
    2.  **CodeGen**: 导出为 LLVM IR。
    3.  **Execution**: LLVM JIT 编译为机器码并执行。

*   **路径 B: GPU 渲染 (RHI)**
    1.  **Lowering**: 将 `math.add` 转换为 `spirv.FAdd`。
    2.  **Lowering**: 将 `rhi.draw` 转换为对应的 Vulkan/DX12 API 调用序列（或者生成 CommandBuffer 构建逻辑）。
    3.  **CodeGen**: 导出为 SPIR-V 二进制。
    4.  **Execution**: 发送给显卡驱动。

---

### 5. 关键设计细节：如何避免耦合？

#### 5.1. 接口 (Interfaces)
IRSystem 定义通用的接口类，例如 `LoopLikeInterface` (像循环一样的东西)。
*   `scf.for` (结构化控制流) 实现它。
*   `rhi.dispatch` (计算着色器调度) 也可以实现它。
*   **Pass 的写法**: Pass 不依赖具体的 Op 类，只依赖 Interface。
    *   `if (auto loop = dyn_cast<LoopLikeInterface>(op)) ...`
    *   这样，一个“循环展开 Pass”可以同时作用于 CPU 循环和 GPU 调度，而不需要知道它们具体是谁。

#### 5.2. 转换方言 (Conversion Dialect)
不要让 Math 依赖 RHI。如果需要转换，创建一个独立的库 `MathToRHI`。
*   这个库包含将 `Math` Op 转换为 `RHI` Op 的 Pattern。
*   只有在最终编译管线组装时，才链接这个库。

### 6. 总结：通用 IR 系统的本质

这个系统实际上是一个**“元编译器”**。

1.  **IRSystem**: 提供画布（Graph）、画笔（Builder）和橡皮擦（Rewriter）。
2.  **Math/RHI**: 定义画布上的符号（Op）和语法（Verifier）。
3.  **Algorithm**: 定义如何把一种符号画成另一种符号（Pattern）。

**它之所以通用，是因为它放弃了理解语义，只负责管理结构。** 语义的理解被下放到了各个 Dialect 的 Pattern 实现中。这就是为什么它能统一 AOT、JIT、Math 和 RHI 的根本原因。