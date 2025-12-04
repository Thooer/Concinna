# Cap.Math 设计文档（Rust 迁移版）

## 核心公理
- Storage != Compute：存储层与计算层分离，类型稳定，算法可激进。
- Batch is King：单实例优化意义有限，批处理 (SoA/Packet) 才能榨干硬件。
- LWC Native：世界坐标使用 `f64`，局部计算使用 `f32`。

## 模块分层
- Storage Layer：`Vec3/Vec4/Quat/Mat4/DVec3` 等标准布局类型，ABI 稳定，序列化/GPU 友好。
- Compute Layer：面向寄存器驻留的批处理算法，提供 SIMD 加速与标量回退。
- Data Stream Layer：SoA 数据结构与批处理接口，连接 ECS/Physics 的数据流水线。

## 已实现内容（本次迁移）
- 类型：`Vec2/Vec3/Vec4/Quat/Mat4/DVec3`（存储层），`Vec3SoA/Vec4SoA`（数据流层）。
- 接口分离：`API` 仅声明，`Impl` 提供 `Windows`（SIMD）与 `Generic`（标量）后端。
- 批处理：`vec4_add_batch`（AoS）、`vec4_add_soa/vec3_dot_soa`（SoA）。
- LWC 转换：`lwc_sub_and_cast(world: &[DVec3], cam: DVec3) -> Vec3SoA`，支持相机相对转换。
- 矩阵：`Mat4::transpose`，`mat4_mul` 优化为先转置 B，降低列构造开销。
- 测试：精度测试（归一化/仿射逆），性能测试（AoS vs SIMD，SoA SIMD）。

## 设计要点（迁移版）

- Storage 与 Compute 分离：存储类型仅负责布局与 ABI；计算逻辑在后端以批处理形式实现。
- 原生 SoA：在批量场景中以切片视图 `Vec3SoAView/Vec4SoAView` 作为主接口，提升缓存命中与吞吐。
- 指令抽象：统一通过 `Prm.SIMD` 原语（`load/store/add/mul/fma/min/max/select/cmp`）实现跨平台。
- 标量回退：所有计算接口提供标量后端，保证可移植性与调试与一致性验证。

## 视图与流式内核

视图是非拥有的 SoA 形态（结构化切片），计算内核处理视图以实现批量 SIMD。

```rust
// API/Views.rs
pub struct Vec3SoAView<'a> { pub x: &'a [f32], pub y: &'a [f32], pub z: &'a [f32] }
pub struct Vec3SoAMutView<'a> { pub x: &'a mut [f32], pub y: &'a mut [f32], pub z: &'a mut [f32] }
```

计算内核示例（Windows 后端，SIMD 加速；非 Windows 使用标量后端）：

```rust
// Impl/Windows/Ops.rs
pub fn impl_vec3_fma_view(a: Vec3SoAView, b: Vec3SoAView, dt: f32, out: Vec3SoAMutView) -> usize { /* SSE/FMA 批处理 */ }
// Impl/Generic/Ops.rs
pub fn impl_vec3_fma_view(a: Vec3SoAView, b: Vec3SoAView, dt: f32, out: Vec3SoAMutView) -> usize { /* 标量回退 */ }
```

API 入口统一分发：

```rust
// API/Ops.rs
pub fn vec3_fma_view(a: Vec3SoAView, b: Vec3SoAView, dt: f32, out: Vec3SoAMutView) -> usize;
```

## Packet 抽象（寄存器驻留）

为了表达“寄存器驻留”的计算单元，提供轻量封装：

```rust
// API/Packet.rs
pub struct Packet4 { pub reg: prm_simd::F32x4 }
impl Packet4 { /* load/store/add/mul/fma 内联封装 */ }
```

## LWC（大世界坐标）

- 存储层提供 `DVec3`；计算层提供 `lwc_sub_and_cast(world: &[DVec3], cam: DVec3) -> Vec3SoA`，实现相机相对坐标转换。
- 后续将扩展为视图版本与批量转换 Kernel，并按平台提供特化优化。

## 矩阵运算优化

- `Mat4::transpose` 提供列/行转置。
- `mat4_mul` 使用“先转置再行点乘”的策略，避免在内层循环中逐列构造导致的混洗与加载开销。

## 验证与基准

- 精度：归一化与仿射逆验证，输出最大误差统计。
- 性能：对比 AoS 标量/ SIMD、SoA SIMD，以及视图内核 `vec3_fma_view` 的批处理吞吐。
- 样例入口：`Foundation/Cap/Math/Sample/Smoke/main.rs`。

## 扩展规划

- Packet：在 `Cap.Math` 中引入通用的 `Packet<T, const W: usize>`，统一内核接口（W=4/8/16）。
- Transcendental：实现向量化 `sin/cos/atan2/pow`（Minimax 多项式逼近）。
- 几何：`Plane/Ray/AABB/Frustum` 批量相交与裁剪（SoA/Packet 内核）。
- LWC 批量转换：视图版本的 `SubAndCast` 与尾处理策略（Padding 或 Masked Store）。
- SoA 宏：过程宏生成 SoA 版本与视图代理，降低样板与手动错误。

## TODO（P2 微优化与扩展）
- 在多项式内核中强制使用 `fma`，统一 `prm_simd` 抽象调用
- 引入 `AlignedVec<T, 32>` 或统一对齐容器以提升内存访问效率
- 扩展 LWC 视图的批量转换与掩码存储策略
- 几何库：`Plane/Ray/AABB/Frustum` 的 SoA/Packet 内核与批量裁剪
- 视图与 SoA 的 Padding 规范化，消除标量尾部循环

## 规划与扩展
- Packet 抽象：在 `Cap.Math` 中引入 `Packet<T, W>`，让寄存器驻留成为一等公民（W=4/8/16）。
- Transcendental：实现向量化 `sin/cos/atan2/pow`（基于 Minimax 多项式）。
- 几何库：`Plane/Ray/AABB/Frustum` 以及批量相交测试与裁剪。
- LWC 批量转换：`SubAndCast` 扩展至 SoA 输入与掩码处理，支持尾部策略。
- SoA 宏工具：过程宏生成 SoA 版本与视图代理，降低样板代码。

## 验证策略
- 精度：标量结果与 SIMD 结果进行 `epsilon` 比较，输出最大误差统计。
- 性能：统一样例基准，输出同规模下 AoS 并行、SoA SIMD 的耗时对比。

## 依赖与边界
- 依赖：`prm_simd` 提供跨平台 SIMD 原语；不直接使用底层 intrinsics。
- 边界：Math 专注算法与数据流，SIMD 专注指令封装；两者职责清晰。

