# Math 模块 API 使用指南

## 核心设计原则

### 三层架构
1. **Storage Layer (存储层)**: POD 类型 + SoA 容器,稳定 ABI,GPU 友好
2. **Compute Layer (计算层)**: Packet 向量化算子,寄存器驻留
3. **LWC Layer (混合精度层)**: 大世界坐标转换,批量精度降级

### 数据流模式
```cpp
// 错误用法: 逐元素标量处理
for (int i = 0; i < count; ++i) {
    Vector3 v = positions[i];
    v = Normalize(v); // 极慢!
    positions[i] = v;
}

// 正确用法: SoA 批量处理
for (USize i = 0; i < count; i += 8) {
    auto packet = LoadPacketAligned<8>(positions, i);
    auto normalized = Vector3Packet<8>::Normalize(packet);
    StorePacketAligned(positions, i, normalized);
}
```

## 存储层 API (Math::Storage)

### 基础类型
```cpp
using Scalar = Float32;  // 局部坐标/旋转
using Real = Float64;    // 世界坐标
using HalfT = Half;      // 存储压缩

struct Vector3 { Scalar x, y, z; };       // 12 bytes, StandardLayout
struct DVector3 { Real x, y, z; };        // 24 bytes, LWC 专用
struct Quaternion { Scalar x, y, z, w; }; // 16 bytes, 旋转
struct Matrix4x4 { Scalar m[16]; };       // 64 bytes, 列主序
```

### SoA 容器模板
```cpp
template<USize BlockSize = 1024>
struct SoAChunkVector3 {
    alignas(64) Scalar xs[BlockSize];  // cache-line 对齐
    alignas(64) Scalar ys[BlockSize];
    alignas(64) Scalar zs[BlockSize];
};
// BlockSize 必须是 16 的倍数
// 推荐值: 256/512/1024/2048
```

### 精度转换
```cpp
Vector3 ToFloat(const DVector3& d);  // Double -> Float
Scalar HalfToScalar(HalfT h);        // Half -> Float
HalfT ScalarToHalf(Scalar f);        // Float -> Half
```

## 计算层 API (Math::Compute)

### Vector3Packet 核心操作
```cpp
// 基础算术 (Width W = 1/4/8)
Vector3Packet<W> Add(Vector3Packet a, Vector3Packet b);
Vector3Packet<W> Sub(Vector3Packet a, Vector3Packet b);
Vector3Packet<W> Mul(Vector3Packet a, Packet<float,W> scalar);

// 几何计算
Packet<float,W> Dot(Vector3Packet a, Vector3Packet b);     // 点积
Vector3Packet<W> Cross(Vector3Packet a, Vector3Packet b);  // 叉积
Packet<float,W> Length(Vector3Packet a);                   // 长度(优化Rsqrt)
Vector3Packet<W> Normalize(Vector3Packet a);               // 零安全归一化

// 高阶算子
Vector3Packet<W> Lerp(Vector3Packet a, Vector3Packet b, Packet<float,W> t);
Vector3Packet<W> Min/Max/Clamp(Vector3Packet v, ...);
Vector3Packet<W> Abs(Vector3Packet a);

// 批量线性代数
Vector3Packet<W> Project(Vector3Packet v, Vector3Packet n);   // 投影
Vector3Packet<W> Reject(Vector3Packet v, Vector3Packet n);    // 排斥
Vector3Packet<W> ClampLength(Vector3Packet v, Packet<float,W> maxLen);
Vector3Packet<W> Reflect(Vector3Packet v, Vector3Packet n);   // 反射
```

### QuaternionPacket 旋转操作
```cpp
Vector3Packet<W> RotateVector(QuaternionPacket q, Vector3Packet<W> v);
QuaternionPacket<W> Conjugate(QuaternionPacket q);
QuaternionPacket<W> Multiply(QuaternionPacket a, QuaternionPacket b);
Packet<float,W> Dot(QuaternionPacket a, QuaternionPacket b);
QuaternionPacket<W> Normalize(QuaternionPacket q);
QuaternionPacket<W> Slerp(QuaternionPacket a, QuaternionPacket b, Packet<float,W> t);
```

### MatrixPacket 变换矩阵
```cpp
MatrixPacket<W> Compose(Vector3Packet<W> pos, QuaternionPacket<W> rot, Vector3Packet<W> scale);
// 输出列主序 4x4 矩阵,直接用于 GPU Uniform
```

### Load/Store 变体
```cpp
// 对齐访问 (需要 cache-line 对齐)
Vector3Packet<W> LoadPacketAligned<W>(const SoAChunkVector3<>& soa, USize i);
void StorePacketAligned<W>(SoAChunkVector3<>& soa, USize i, Vector3Packet<W> p);

// 非对齐访问 (安全但慢)
Vector3Packet<W> LoadPacketUnaligned<W>(const SoAChunkVector3<>& soa, USize i);
void StorePacketUnaligned<W>(SoAChunkVector3<>& soa, USize i, Vector3Packet<W> p);

// 掩码访问 (尾部处理)
Vector3Packet<W> LoadPacketMaskedAligned<W>(..., USize count);
void StorePacketMaskedUnaligned<W>(..., USize count);
```

## LWC 混合精度 API (Math::LWC)

### Camera-Relative 转换
```cpp
// 单点转换
Vector3 SubAndCast(const DVector3& world_pos, const DVector3& cam_pos);

// 批量标量转换
void BatchConvert(const SoAChunkDVector3<>& world,
                  const DVector3& camera,
                  SoAChunkVector3<>& local,
                  USize count);

// 批量 SIMD 转换 (推荐)
void BatchConvertPacket4(const SoAChunkDVector3<>& world,
                         const DVector3& camera,
                         SoAChunkVector3<>& local,
                         USize count);
// 利用 AVX vcvtpd2ps 指令,比标量版快 3-4x
```

## 性能优化建议

### 1. 对齐策略
```cpp
// 好: 内存布局对齐到 64 字节
SoAChunkVector3<1024> positions;  // BlockSize % 16 == 0
static_assert(alignof(decltype(positions)) == 64);

// 坏: 动态分配未对齐
Vector3* data = new Vector3[count];  // AoS 布局,无法向量化
```

### 2. 宽度选择
- **W=8 (AVX2)**: 主循环,需要 AVX2 支持
- **W=4 (SSE)**: 回退路径,兼容性好
- **W=1 (Scalar)**: 调试/标量后端

### 3. 尾部处理
```cpp
USize i = 0;
for (; i + 8 <= count; i += 8) {
    // 主循环: 对齐 Load/Store
}
if (i < count) {
    // 尾部: 掩码或标量回退
    auto tail = LoadPacketMaskedUnaligned<8>(soa, i, count - i);
    // ...
}
```

### 4. 数值稳定性
```cpp
// Normalize 自带零检测
auto norm = Vector3Packet<W>::Normalize(v);  // 零向量 -> (0,0,0)

// Slerp 退化到 Lerp
auto slerp = QuaternionPacket<W>::Slerp(a, b, t);  // dot > 0.9995 线性回退
```

## 典型流水线示例

### ECS Transform 系统
```cpp
void UpdateTransforms(SoAChunkVector3<>& positions,
                      SoAChunkQuaternion<>& rotations,
                      SoAChunkVector3<>& scales,
                      SoAChunkMatrix4x4<>& matrices,
                      USize count) {
    constexpr int W = 8;
    for (USize i = 0; i < count; i += W) {
        auto pos = LoadPacketAligned<W>(positions, i);
        auto rot = LoadPacketAligned<W>(rotations, i);
        auto scl = LoadPacketAligned<W>(scales, i);
        
        auto mat = MatrixPacket<W>::Compose(pos, rot, scl);
        StorePacketAligned(matrices, i, mat);
    }
}
```

### 大世界坐标转换
```cpp
void ConvertToLocalSpace(const SoAChunkDVector3<>& worldPos,
                         const DVector3& cameraPos,
                         SoAChunkVector3<>& localPos,
                         USize count) {
    LWCConverter::BatchConvertPacket4(worldPos, cameraPos, localPos, count);
}
```

### 粒子系统速度约束
```cpp
void ClampParticleVelocities(SoAChunkVector3<>& velocities,
                             float maxSpeed,
                             USize count) {
    constexpr int W = 8;
    auto maxLen = SIMD::Set1<W>(maxSpeed);
    for (USize i = 0; i < count; i += W) {
        auto v = LoadPacketAligned<W>(velocities, i);
        auto clamped = Vector3Packet<W>::ClampLength(v, maxLen);
        StorePacketAligned(velocities, i, clamped);
    }
}
```

## 调试技巧

### Natvis 可视化
```xml
<!-- .natvis 文件 -->
<Type Name="Math::Vector3Packet&lt;8&gt;">
  <DisplayString>[0]={{xs[0],ys[0],zs[0]}}, ...</DisplayString>
</Type>
```

### 标量验证
```cpp
#ifdef MATH_DEBUG_SCALAR
    #define PACKET_WIDTH 1  // 强制标量路径
#else
    #define PACKET_WIDTH 8
#endif
```

### 精度比较
```cpp
// Epsilon 容忍 FMA 重排
That(result, ctx).ToBeApproximately(expected, 1e-5f);
```

## API 稳定性保证

- **Storage Layer**: ABI 永久锁定,`static_assert` 验证
- **Compute Layer**: 接口稳定,可添加新算子
- **LWC Layer**: 批量接口稳定,可优化实现

版本: v1.0 (Phase 3 完成)  
最后更新: 2025-11-24
