# Engine.Math 模块设计文档 V3.1 (Progressive -> Radical)

## 0. 核心公理 (Axioms)

1. **Storage != Compute**: 存储层的数据结构与计算层的数据结构是两种完全不同的形态。
2. **Batch is King**: 单个向量的 SIMD 优化是徒劳的；只有批量处理 (Packet/SoA) 才能榨干 CPU 性能。
3. **LWC Native**: 世界坐标永远是 `Double`，局部计算永远是 `Float`。

## 1. 核心设计哲学 (Core Philosophy)

本模块不仅仅是数学公式的集合，而是引擎的 **数据流处理核心 (Data Stream Processing Core)**，采用 **渐进式激进设计 (Progressive Radical Design)**。

1.  **数据驱动架构 (Data-Driven)**：从底层存储抽象到高性能计算的分层设计
2.  **Batch Processing 优先**: 大规模数据批处理比单实例运算更重要
3.  **严格分层 (Strict Layering)**：
    *   **存储层 (Storage Layer)**: 稳定的 ABI，GPU 友好，序列化友好
    *   **计算层 (Compute Layer)**: 激进的 SIMD 利用，寄存器驻留计算
    *   **数据流层 (Data Stream Layer)**: 连接 ECS 和 Physics 的 SoA 数据流
4.  **混合精度 (Mixed Precision)**：
    *   **LWC (Large World Coordinates)**: 世界空间使用 `Double`
    *   **Local**: 局部空间/渲染使用 `Float`
    *   **Storage**: 海量数据存储使用 `Half`

## 2. 模块依赖 (Dependencies)

*   **Import `Language`**: 使用基础类型、概念、内存工具等核心特性
*   **Import `SIMD`**: **关键依赖**。获取底层的 Intrinsic 包装和向量化指令支持
*   **Import `System`**: 获取 CPU 特性检测和硬件能力评估

## 3. 架构分层详解 (Architecture Layers)

### Phase 1: 基础存储层 (The Storage Layer)
**目标**: 稳定的 ABI，GPU 友好，序列化友好。**绝不**在此层引入 SIMD 类型成员。

#### 3.1 标量定义
利用 `Language:Types` 确保跨平台尺寸。
*   `Scalar` = `float` (32-bit) -> 用于局部坐标、旋转、缩放、颜色。
*   `Real` = `double` (64-bit) -> 仅用于 **World Position**。
*   `Half` = `uint16_t` -> 仅用于海量数据存储 (Mesh/Texture)。

#### 3.2 复合类型 (POD)
这些结构体必须是 `StandardLayout`，无填充，无虚函数。
```cpp
// 内存布局：XYZ (12 bytes)
struct Vector3 { Scalar x, y, z; }; 

// 内存布局：XYZ (24 bytes) - LWC 专用
struct DVector3 { Real x, y, z; };

// 内存布局：XYZW (16 bytes)
struct Quaternion { Scalar x, y, z, w; };

// 内存布局：16 floats (64 bytes), 列主序 (Column-Major)
struct Matrix4x4 { Scalar m[16]; };
```

#### 3.3 基础转换
*   提供 `Load/Store` 辅助函数，将 `Half` 转为 `Scalar`。
*   提供 `ToFloat(DVector3)` -> `Vector3` (相对于 Camera 的转换)。

### Phase 2: 宽计算层 (The Compute Layer)
**目标**: 激进的 SIMD 利用。这是数学库的**真正形态**。
**原则**: 数据一旦进入 CPU 寄存器，就必须保持在寄存器中，直到计算结束。

#### 3.4 Packet 抽象 (寄存器驻留)
利用 `SIMD` 模块封装底层指令。`W` 是宽度 (Width)，通常是 4 (SSE), 8 (AVX), 16 (AVX512)。

```cpp
// 这是一个"逻辑"类型，实际上它持有 SIMD 寄存器
template<typename T, int W>
struct Packet;

// 特化：AVX2 Float Packet
template<> struct Packet<float, 8> {
    __m256 reg; // 直接持有寄存器类型
    
    FORCEINLINE Packet operator+(Packet rhs) const {
        return { _mm256_add_ps(reg, rhs.reg) };
    }
    // ... Mul, FMA, Sub
};
```

#### 3.5 混合 SoA 向量 (Hybrid SoA Vector)
这是**最激进**的设计点：在计算层，`Vector3` 不是 `[x,y,z]`，而是 `[xxxx..., yyyy..., zzzz...]`。

```cpp
// 同时处理 8 个 Vector3
struct Vector3Packet<8> {
    Packet<float, 8> xs; // 寄存器 YMM0
    Packet<float, 8> ys; // 寄存器 YMM1
    Packet<float, 8> zs; // 寄存器 YMM2

    // 纯寄存器操作，吞吐量极大
    FORCEINLINE static Vector3Packet Add(Vector3Packet a, Vector3Packet b) {
        return { a.xs + b.xs, a.ys + b.ys, a.zs + b.zs };
    }
    
    // Dot Product: 结果也是一个 Packet (8个点积结果)
    FORCEINLINE static Packet<float, 8> Dot(Vector3Packet a, Vector3Packet b) {
        // FMA: x*x + y*y + z*z
        auto r = a.xs * b.xs;
        r = FMA(a.ys, b.ys, r);
        r = FMA(a.zs, b.zs, r);
        return r;
    }
};
```

### Phase 3: 激进的数据流 (The Radical Data Stream)
**目标**: 连接 Storage 和 Compute。这是 ECS 和 Physics 的核心接口。

#### 3.6 SoA 容器 (The Truth)
ECS 的 Component 数据**必须**以 SoA 形式存储，以便直接加载到 `Vector3Packet`。

```cpp
// 内存中的形态：
// Chunk: [x,x,x,x... y,y,y,y... z,z,z,z...]
template<typename T, size_t BlockSize = 1024>
struct SoAChunk {
    alignas(64) float xs[BlockSize];
    alignas(64) float ys[BlockSize];
    alignas(64) float zs[BlockSize];
};
```

#### 3.7 流式处理 (Stream Kernel)
不要写 `Update(Entity e)`，要写 `UpdateKernel(Stream in, Stream out)`。

```cpp
// 激进的 API 设计
void TransformSystem::Update(const SoAChunk<Transform>& in, SoAChunk<Matrix4x4>& out, size_t count) {
    const size_t W = 8; // AVX2
    
    for (size_t i = 0; i < count; i += W) {
        // 1. Load: 内存 -> 寄存器 (3条指令)
        Vector3Packet<W> pos = LoadPacket(in.pos, i);
        QuatPacket<W>    rot = LoadPacket(in.rot, i);
        Vector3Packet<W> scl = LoadPacket(in.scale, i);

        // 2. Compute: 纯寄存器操作 (无内存延迟)
        MatrixPacket<W> mat = Matrix::Compose(pos, rot, scl);

        // 3. Store: 寄存器 -> 内存 (16条指令，写入流缓冲)
        StorePacket(out.m, i, mat);
    }
}
```

### Phase 4: 辅助与胶水 (Glue & Utils)

#### 3.8 LWC 转换桥梁
在渲染前，必须将 `DVector3` (World) 转为 `Vector3` (Local)。
*   **Camera-Relative**: `LocalPos = (WorldPos - CamPos).ToFloat()`
*   **实现**: 提供 `SubAndCast(DVector3Packet, DVector3Packet) -> Vector3Packet` 的专用指令，利用 AVX 的 `vcvtpd2ps` (Double to Float) 指令。

#### 3.9 强类型单位
利用 C++20 `consteval` 消除运行时开销。
```cpp
struct Radian { float val; };
struct Degree { float val; };

consteval Radian operator""_deg(long double d) {
    return Radian{ static_cast<float>(d * 0.0174533) };
}
// 编译期计算 sin/cos 表
```

#### 3.10 几何图元
*   **PacketFrustum**: 6 个 `PacketPlane`。
*   **PacketAABB**: 8 个 AABB (SoA layout: min_xs, min_ys... max_zs)。
*   **Culling**: `Intersect(PacketFrustum, PacketAABB)` -> 返回 `Mask8` (1 byte)。

## 4. 接口设计规范 (API Specification)

### 4.1. 零开销 Swizzle

利用 C++20 `requires` 和 模板元编程实现零开销 Swizzle。

```cpp
// 仅在编译期计算偏移，无运行时开销
template<int X, int Y, int Z, int W>
struct SwizzleProxy { ... };

Vector4 v;
v.zyx(); // 返回 SwizzleProxy，赋值时才生成 shuffle 指令
```

**实现建议**: 向量本体只是存储，Swizzle 是一个带索引的轻量代理，编译期根据索引展开读写。

### 4.2. 强类型角度 (Strong Typing)

利用 C++20 `consteval` 消除运行时开销。

```cpp
struct Radian { float val; };
struct Degree { float val; };

consteval Radian operator""_deg(long double d) {
    return Radian{ static_cast<float>(d * 0.0174533) };
}
// 编译期计算 sin/cos 表
```

### 4.3. 激进数据流接口

```cpp
// 禁止单个实体的 Update API
// void TransformSystem::Update(Entity e); // NO!

// 强制使用批处理 Stream API
void TransformSystem::UpdateKernel(
    const SoAChunk<Transform>& in, 
    SoAChunk<Matrix4x4>& out, 
    size_t count
);
```

### 4.4. 易用性中间层 (View Layer)

为了解决易用性与性能的博弈问题，提供中间层 API：

```cpp
// 提供 SoAView 或 EntityView，重载操作符，看起来像操作单个对象
struct SoAView {
    Vector3Packet<W>& operator[](size_t index);
    // 实际上操作 SoA 数据流中的一个切片
};

// 或者保留 AoS 到 SoA 的即时转换
Vector3Packet<W> LoadPacket(const Array<Vector3>& aos, size_t index);
void StorePacket(Array<Vector3>& aos, size_t index, Vector3Packet<W> packet);
```

### 4.5. Mask 类型与分支逻辑

在 SIMD 计算中，`if (a > b)` 不存在，需要 Mask 类型：

```cpp
template<int W>
struct PacketMask {
    // 比较操作返回掩码
    static PacketMask<W> GreaterThan(Packet<float, W> a, Packet<float, W> b);
    
    // 无分支选择
    static Packet<float, W> Select(PacketMask<W> mask, Packet<float, W> true_val, Packet<float, W> false_val);
};
```

### 4.6. 超越函数向量化

CPU 指令集通常只提供基础运算，超越函数需要向量化实现：

```cpp
// 基于 Minimax 多项式逼近的向量化超越函数
struct Transcendental {
    static Packet<float, W> Sin(Packet<float, W> x);
    static Packet<float, W> Cos(Packet<float, W> x);
    static Packet<float, W> Atan2(Packet<float, W> y, Packet<float, W> x);
    static Packet<float, W> Pow(Packet<float, W> base, Packet<float, W> exp);
};
```

### 4.7. Load/Store 特定变体

必须明确区分对齐和非对齐操作，防止开发者在未对齐内存上调用对齐加载导致崩溃：

```cpp
struct PacketOps {
    // 对齐加载/存储 (要求 32 字节对齐)
    static Packet<float, W> LoadAligned(const float* ptr);
    static void StoreAligned(float* ptr, Packet<float, W> value);
    
    // 非对齐加载/存储 (安全但较慢)
    static Packet<float, W> LoadUnaligned(const float* ptr);
    static void StoreUnaligned(float* ptr, Packet<float, W> value);
    
    // Gather/Scatter: 从不连续内存加载 (非常慢，但在复杂 ECS 组件关系中可能需要)
    static Packet<float, W> Gather(const float* base, Packet<int, W> indices);
    static void Scatter(float* base, Packet<int, W> indices, Packet<float, W> value);
};
```

### 4.8. LWC 特殊指令优化

针对高频的 DVector3 -> Vector3 转换进行专门优化：

```cpp
struct LWCConverter {
    // 专门优化的 SubAndCast 函数 (Camera-Relative 转换)
    static Vector3Packet<W> SubAndCast(DVector3Packet<W> world_pos, DVector3Packet<W> cam_pos) {
        // AVX2 优化路径: 先 Sub 再 Convert (vcvtpd2ps)
        auto delta = world_pos - cam_pos;
        return ConvertDoubleToFloat(delta);
    }
    
    // 批量转换接口 (每帧每个可见物体都要调用)
    static void BatchConvert(const SoAChunk<DVector3>& world_positions,
                           const DVector3& camera_position,
                           SoAChunk<Vector3>& local_positions,
                           size_t count);
};
```

## 5. 技术挑战与解决方案 (Technical Challenges & Solutions)

### 5.1 易用性 vs. 性能的博弈
*   **问题**: `UpdateKernel(SoAChunk& in, ...)` 这种 API 对游戏逻辑开发者来说太反直觉。
*   **解决方案**: 
*   提供**中间层（View Layer）**，支持 `SoAView` 或 `EntityView`，重载操作符看起来像操作单个对象，但实际上操作 SoA 数据流切片。
*   SoAView 必须是零成本抽象（Zero-overhead abstraction）。
*   大量使用 [[no_unique_address]] (C++20)。
*   Proxy Pattern: view.position 不应该返回 Vector3，而应该返回一个 Vector3Ref 代理对象，重载赋值操作符，将写入操作打散到底层 SoA 数组中。

### 5.2 超越指令集的数学函数
*   **挑战**: CPU 指令集通常只提供加减乘除和平方根，`sin`, `cos`, `atan2`, `pow` 等超越函数没有直接的 SIMD 指令。
*   **解决方案**: 在 `Math` 模块内部实现一套**向量化的数学近似库**，基于 Minimax 多项式逼近（如 Remez 算法）。

### 5.3 尾部处理 (Tail Handling)
*   **问题**: 当数据量不是 `W` 的倍数时，最后剩余的元素处理困难。
*   **解决方案**:
    *   **Padding**: 强制 `SoAChunk` 大小必须是 `W` 的倍数，不足补零（最快且最安全）。
    *   **Masked Store**: 利用 AVX 的 Masked Store 指令处理尾部（较慢但精确）。
*   风险：如果在每个 Kernel 内部都手动写一套 Masked Store 或 Scalar Loop，代码会非常丑陋且难以维护。
*   Padding 策略优先：对于 SoAChunk，强制其容量总是 W 的倍数（例如总是分配 16 的倍数个 float）。哪怕实际只有 3 个实体，内存里也分配 16 个位置。
*   这样 Kernel 循环可以写成 i < aligned_count，完全消除尾部判断分支，用微小的内存浪费换取指令流水线的极度整洁。

### 5.4 编译期 Swizzle 的实现难度
*   **挑战**: 实现零开销 Swizzle 需要复杂的表达式模板或代理对象。
*   **解决方案**: 向量本体只是存储，Swizzle 是带索引的轻量代理，编译期根据索引展开读写。

## 6. 调试与可视化 (Debugging & Visualization)

### 6.1 调试器可视化 (Natvis/Lldbinit)
由于 Vector3Packet 在调试器里看起来就是一堆乱糟糟的 `__m256` 寄存器数据，必须编写调试器可视化文件：

```xml
<!-- Visual Studio .natvis 文件 -->
<AutoVisualizer xmlns="http://schemas.microsoft.com/vstudio/debugger/natvis/2010">
  <Type Name="Vector3Packet<8>">
    <DisplayString>[0] = {{x: {xs[0]}, y: {ys[0]}, z: {zs[0]}}}, [1] = {{x: {xs[1]}, y: {ys[1]}, z: {zs[1]}}}, ...</DisplayString>
    <Expand>
      <Item Name="[0]">xs[0], ys[0], zs[0]</Item>
      <Item Name="[1]">xs[1], ys[1], zs[1]</Item>
      <!-- 显示所有 8 个元素 -->
    </Expand>
  </Type>
</AutoVisualizer>
```

### 6.2 验证策略 (Validation Strategy)
建立标量版本和 Packet 版本的对比验证机制：

```cpp
struct MathValidator {
    // 同一套输入数据，同时跑 Scalar 和 Packet 版本
    template<typename ScalarFunc, typename PacketFunc>
    static bool Validate(ScalarFunc scalar_func, PacketFunc packet_func, 
                        const TestData& inputs, float epsilon = 1e-5f) {
        auto scalar_results = RunScalar(scalar_func, inputs);
        auto packet_results = RunPacket(packet_func, inputs);
        
        // 由于浮点数指令重排，结果不会完全相等，使用 Epsilon 比较
        return CompareWithEpsilon(scalar_results, packet_results, epsilon);
    }
    
    // 专门用于验证超越函数
    static bool ValidateTranscendental(const char* func_name, 
                                     const std::vector<float>& test_cases);
};
```

## 7. 模块边界与职责划分 (Module Boundaries)

### 7.1 Math 与 SIMD 的明确界限
*   **SIMD 模块职责**: 只负责封装 `_mm256_add_ps` 等底层指令，提供统一的 SIMD 抽象层。
*   **Math 模块职责**: 负责实现 `Dot`, `Cross`, `Normalize`, `Sin`, `Cos` 等数学逻辑。
*   **禁止**: 不要让 `SIMD` 模块承担数学逻辑。
*   不同指令集的行为并不总是 1:1 对应的。例如，AVX 的 rsqrt 精度和 NEON 的 rsqrte 精度不同；某些 Shuffle 指令在不同架构下掩码定义完全不同。
*   不要追求 100% 的指令映射。
*   在 Math 层实现算法时，尽量使用通用的原语（如 Min, Max, FMA, Select），避免依赖特定架构的奇技淫巧（如特定的 SSE shuffle mask），否则移植到 ARM/Console 时会非常痛苦。
### 7.2 标量回退后端 (Scalar Fallback)
*   **必要性**: 为了调试和非 SIMD 平台，必须实现 `Packet<T, 1>` 的特化版本。
*   **实现**: 用普通的 `float` 循环模拟 SIMD 行为，便于单元测试验证算法正确性。

## 7. 性能优化策略 (Optimization Strategy)

1.  **Force Inline**: 所有数学函数标记为 `FORCEINLINE`，确保指令融合。
2.  **Register Residency**: 数据一旦进入寄存器，就必须保持到计算结束。
3.  **No Alias**: 关键计算函数的指针参数使用 `RESTRICT`，告知编译器内存不重叠。
4.  **Branchless**: 利用 SIMD 比较指令直接生成掩码，避免 `if` 分支。
5.  **FMA (Fused Multiply-Add)**: 充分利用融合乘加指令。

## 8. 架构图修正建议 (Architecture Diagram Enhancement)

在 Compute Layer 和 Data Stream Layer 之间，显式地画出 "Transform Pipeline"：

```mermaid
graph TD
    SubGraph_Storage[Storage Layer (Memory/Disk)]
    A[Vector3 (POD)] -->|Batch Load| B(SoAChunk)
    
    SubGraph_Compute[Compute Layer (Registers)]
    B -->|LoadPacket| C[Vector3Packet (AVX/NEON)]
    C -->|SIMD Math| D[ResultPacket]
    D -->|StorePacket| B
    
    SubGraph_Logic[Game Logic]
    L[Gameplay Code] -.->|View Proxy| B
    
    SubGraph_Transform[Transform Pipeline]
    B -->|LoadAligned| T1[Position Packet]
    B -->|LoadAligned| T2[Rotation Packet]
    B -->|LoadAligned| T3[Scale Packet]
    T1 -->|Compose| T4[Matrix Packet]
    T2 -->|Compose| T4
    T3 -->|Compose| T4
    T4 -->|StoreAligned| B
```

## 9. Foundation 服务化目标契合度 (Foundation Service Alignment)

### 9.1 无依赖性
*   **优势**: Math 模块只依赖 `SIMD` 抽象，完全独立，符合 Foundation 层"为外部服务"的目标。

### 9.2 通用性设计
*   **简单工具**: 对于外部工具，可以使用 Phase 1 的 `Vector3` (POD)，简单直接。
*   **高性能系统**: 对于物理引擎或粒子系统，可以使用 Phase 3 的 `SoAChunk` 和 `Packet`。

### 9.3 ABI 稳定性
*   **跨模块边界**: 通过将复杂计算逻辑剥离出数据结构，`Vector3` 等基础类型的内存布局可以保持永久不变。
*   **数据交换格式**: 非常适合作为跨模块边界的数据交换格式。

## 10. 测试与验证 (Testing)

1.  **单元测试**: 覆盖所有基础运算和边界条件。
2.  **精度验证**: 对比不同精度级别的计算结果。
3.  **性能基准 (Benchmark)**:
    *   对比 `Array<Vector3>` vs `SoA<Vector3>` 在 100k 规模下的遍历性能。
    *   对比 `Vector3Packet` vs 单个 `Vector3` 的批处理性能。
    *   测试 LWC 转换的性能开销。

## 11. 执行计划 (Execution Plan)

### 11.1 渐进式实施路径

**第一步**: 仅实现 **Phase 1 (存储层)**。让引擎能跑起来，能存盘，能渲染（哪怕慢）。

**第二步**: 实现 `SIMD` 模块的基础包装，集成到 Math 中。

**第三步**: 实现 **Phase 2 (计算层)** 的 `Packet<float, 8>` 和 `Vector3Packet`。

**第四步 (激进点)**: 重构 ECS/Transform 组件，强制使用 **Phase 3 (数据流层)** 的 SoA 布局。此时性能会发生质变。

### 11.2 关键决策

**不要试图让 `Vector3` 既是存储又是计算。** `Vector3` 是数据，`Vector3Packet` 是算力。**分裂它们**。

## 12. 可移植性与工程细节 (Portability & Engineering)

### 12.1 指令集策略与回退
- 提供强制的标量回退路径，语义与 Packet 实现保持一致。
- 在 x86 上优先使用 `SSE2/AVX2/AVX512`，在 ARM 上提供 `NEON` 实现。
- 运行时指令集检测由 `System` 模块提供，编译期通过开关控制可用后端。
- 所有后端通过统一的 `SIMD` 抽象层暴露，禁止直接使用 intrinsics。

### 8.2 宽度与尾处理 (W 与 Tail)
- `W` 按硬件能力选择并在内核中作为常量使用。
- 对 `count % W != 0` 的尾部元素，采用掩码加载/存储或回退标量循环处理。
- `LoadPacket/StorePacket` 明确支持对齐与非对齐数据，两者行为一致且无 UB。
- 非对齐场景应尽量在数据流层进行对齐修复，计算层不依赖未定义对齐。

### 8.3 内存带宽与预取/流写
- 为 SoA 布局提供可调的预取距离与流式写策略，减少回写压力。
- 在 NUMA 环境下，按 `SoAChunk` 分配与绑定节点，避免跨节点访问。
- 输出缓冲区按 64 字节对齐，减少 false sharing 与 cache line 抖动。

### 8.4 调试与验证模式
- 提供全局开关将 Packet 路径降级为标量实现，便于单步调试与问题定位。
- 验证模式下记录关键计算的输入/输出快照，用于与标量结果比对。
- 允许在构建中关闭 `FMA`，用于数值可重复性验证。

### 8.5 ABI/序列化规范
- 所有存储层类型保持 `StandardLayout` 与稳定 ABI，不引入虚函数或隐式填充。
- 明确字节序为小端，统一 `alignas(16)` 对齐策略，跨平台一致。
- `Half` 使用 IEEE 754 半精度编码；序列化加入 schema 版本号确保兼容。
- `Matrix4x4` 以列主序序列化，兼容 GPU API。

### 8.6 SIMD 依赖管理
- 所有指令封装集中于 `SIMD` 模块，统一处理编译器差异与目标特性。
- 通过功能探测选择后端，保证无可用 SIMD 时自动回退到标量路径。

### 8.7 数值稳定性与可重复性
- 在不同后端与 `FMA` 开关状态下进行差异对比测试，量化误差界限。
- 明确舍入模式使用 `round-to-nearest, ties-to-even`，禁用非确定性路径。
- 对 LWC 混合精度计算，给出误差传播与边界用例的覆盖测试。

### 8.8 并发与写放大
- `SoAChunk` 写入采用分块并行与按块对齐，减少写放大与 cache 争用。
- 对跨块处理，避免在同一 cache line 上进行多线程写入。

### 8.9 稀疏访问策略 (Gather/Scatter/Prefetch)
- 在关键路径禁止稀疏 gather；必须时使用专用 gather 后端并记录性能影响。
- 对 scatter 操作使用掩码存储并评估带宽成本；必要时改用压缩缓冲。
- 提供轻量预取 API，避免在短循环中滥用预取指令。

### 8.10 SoAChunk 参数化与自动调优
- `BlockSize` 支持构建时/运行时参数化，默认 `1024`。
- 在持续集成中针对不同 `BlockSize` 进行吞吐与延迟基准测试，选择最优配置。

### 8.11 LWC 转换量化
- 为 `DVector3 -> Vector3` 的批量转换编写 microbench，记录每帧开销。
- 优先采用 Camera-Relative 转换，必要时引入结果缓存以降低重复转换成本。

### 8.12 构建与运行时配置
- 构建选项示例：`MATH_ENABLE_SIMD`、`MATH_ENABLE_NEON`、`MATH_FORCE_SCALAR`、`MATH_DEFAULT_BLOCKSIZE`。
- 运行时通过 `System` 的 CPU 特性检测选择最佳后端，无法匹配时回退标量。

### 8.13 修订后的实施顺序
- 先实现可移植的 SIMD 抽象与标量回退（含 NEON）。
- 完成尾处理与对齐策略，统一 `LoadPacket/StorePacket` 语义。
- 提供调试/验证模式与 FMA 开关，支撑单元测试与差异对比。
- 量化 LWC 转换成本并决定是否引入缓存或惰性转换。
- 明确 ABI/序列化规范与版本号，保障资产兼容。
- 增加数值可重复性测试（跨架构与 FMA 开/关）。
- 参数化与自动调优 `SoAChunk::BlockSize`，在 CI 上收集基准。
- 设计并限制 Gather/Scatter/Prefetch 在关键路径的使用范围。
