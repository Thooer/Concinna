module;

import <cmath>;
import <limits>;

export module ExtendedMathTest;

import Language;
import Test;
import Math;
import SIMD;

using namespace Math;
using namespace SIMD;
using namespace Test;

// ==================== 极值与退化场景测试 ====================

static void Vector3_ExtremeValues_MinMax(ITestContext& ctx) noexcept {
    constexpr float kMax = 1e20f;
    constexpr float kMin = -1e20f;
    constexpr float kTiny = 1e-20f;
    
    auto vMax = Vector3Packet<1>{ Packet<float,1>{kMax}, Packet<float,1>{kMax}, Packet<float,1>{kMax} };
    auto vMin = Vector3Packet<1>{ Packet<float,1>{kMin}, Packet<float,1>{kMin}, Packet<float,1>{kMin} };
    auto vTiny = Vector3Packet<1>{ Packet<float,1>{kTiny}, Packet<float,1>{kTiny}, Packet<float,1>{kTiny} };
    
    auto minRes = Vector3Packet<1>::Min(vMax, vMin);
    That(minRes.xs.reg, ctx).ToBeApproximately(kMin, 1.0f);
    
    auto maxRes = Vector3Packet<1>::Max(vMax, vMin);
    That(maxRes.xs.reg, ctx).ToBeApproximately(kMax, 1.0f);
    
    auto tinyLen = Vector3Packet<1>::Length(vTiny);
    That(tinyLen.reg > 0.0f, ctx).ToBe(true);
}

static void Vector3_NearZero_Normalize(ITestContext& ctx) noexcept {
    auto nearZero = Vector3Packet<1>{ 
        Packet<float,1>{1e-10f}, 
        Packet<float,1>{1e-10f}, 
        Packet<float,1>{1e-10f} 
    };
    auto norm = Vector3Packet<1>::Normalize(nearZero);
    auto lenSq = Vector3Packet<1>::LengthSquared(norm);
    That(lenSq.reg, ctx).ToBeApproximately(1.0f, 1e-3f);
}

static void Quaternion_Identity_Invariance(ITestContext& ctx) noexcept {
    auto identity = QuaternionPacket<1>{ 
        Packet<float,1>{0.0f}, Packet<float,1>{0.0f}, 
        Packet<float,1>{0.0f}, Packet<float,1>{1.0f} 
    };
    auto mul = QuaternionPacket<1>::Multiply(identity, identity);
    That(mul.ws.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
    That(mul.xs.reg, ctx).ToBeApproximately(0.0f, 1e-6f);
    
    auto conj = QuaternionPacket<1>::Conjugate(identity);
    That(conj.ws.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
}

static void Quaternion_Slerp_SameQuaternions(ITestContext& ctx) noexcept {
    float s = 0.70710678f;
    auto q = QuaternionPacket<1>{ 
        Packet<float,1>{0.0f}, Packet<float,1>{0.0f}, 
        Packet<float,1>{s}, Packet<float,1>{s} 
    };
    auto slerp = QuaternionPacket<1>::Slerp(q, q, Packet<float,1>{0.5f});
    auto norm = QuaternionPacket<1>::Normalize(slerp);
    That(norm.zs.reg, ctx).ToBeApproximately(s, 1e-4f);
    That(norm.ws.reg, ctx).ToBeApproximately(s, 1e-4f);
}

static void Matrix_Compose_UnitScale_Identity(ITestContext& ctx) noexcept {
    auto identity = QuaternionPacket<1>{ 
        Packet<float,1>{0.0f}, Packet<float,1>{0.0f}, 
        Packet<float,1>{0.0f}, Packet<float,1>{1.0f} 
    };
    auto zero = Vector3Packet<1>{ 
        Packet<float,1>{0.0f}, Packet<float,1>{0.0f}, Packet<float,1>{0.0f} 
    };
    auto one = Vector3Packet<1>{ 
        Packet<float,1>{1.0f}, Packet<float,1>{1.0f}, Packet<float,1>{1.0f} 
    };
    auto m = MatrixPacket<1>::Compose(zero, identity, one);
    
    That(m.c0x.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
    That(m.c1y.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
    That(m.c2z.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
    That(m.c3w.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
}

// ==================== 数值稳定性测试 ====================

static void LWC_Precision_LargeWorldCoordinates(ITestContext& ctx) noexcept {
    constexpr double kWorldOrigin = 1e12;
    SoAChunkDVector3<16> world{};
    for (USize i = 0; i < 8; ++i) {
        world.xs[i] = kWorldOrigin + static_cast<double>(i) * 0.01;
        world.ys[i] = kWorldOrigin + static_cast<double>(i) * 0.02;
        world.zs[i] = kWorldOrigin + static_cast<double>(i) * 0.03;
    }
    
    DVector3 cam{ kWorldOrigin, kWorldOrigin, kWorldOrigin };
    SoAChunkVector3<16> local{};
    LWCConverter::BatchConvertPacket4(world, cam, local, 8);
    
    for (USize i = 0; i < 8; ++i) {
        That(local.xs[i], ctx).ToBeApproximately(static_cast<float>(i) * 0.01f, 1e-5f);
        That(local.ys[i], ctx).ToBeApproximately(static_cast<float>(i) * 0.02f, 1e-5f);
        That(local.zs[i], ctx).ToBeApproximately(static_cast<float>(i) * 0.03f, 1e-5f);
    }
}

static void Half_Precision_Boundary_Values(ITestContext& ctx) noexcept {
    constexpr float kHalfMax = 65504.0f;
    constexpr float kHalfMin = -65504.0f;
    
    auto hMax = ScalarToHalf(kHalfMax);
    auto hMin = ScalarToHalf(kHalfMin);
    
    auto fMax = HalfToScalar(hMax);
    auto fMin = HalfToScalar(hMin);
    
    That(fMax, ctx).ToBeApproximately(kHalfMax, 100.0f);
    That(fMin, ctx).ToBeApproximately(kHalfMin, 100.0f);
}

// ==================== 批量线性代数扩展 ====================

static void Vector3Packet_BatchProjection(ITestContext& ctx) noexcept {
    auto v = Vector3Packet<4>{ 
        Packet<float,4>::LoadUnaligned((float[]){1.0f, 2.0f, 3.0f, 4.0f}),
        Packet<float,4>::LoadUnaligned((float[]){0.0f, 0.0f, 0.0f, 0.0f}),
        Packet<float,4>::LoadUnaligned((float[]){0.0f, 0.0f, 0.0f, 0.0f})
    };
    auto n = Vector3Packet<4>{ 
        Packet<float,4>::LoadUnaligned((float[]){1.0f, 1.0f, 1.0f, 1.0f}),
        Packet<float,4>::LoadUnaligned((float[]){0.0f, 0.0f, 0.0f, 0.0f}),
        Packet<float,4>::LoadUnaligned((float[]){0.0f, 0.0f, 0.0f, 0.0f})
    };
    
    auto proj = Vector3Packet<4>::Project(v, n);
    
    float projX[4];
    Packet<float,4>::StoreUnaligned(projX, proj.xs);
    That(projX[0], ctx).ToBeApproximately(1.0f, 1e-6f);
    That(projX[1], ctx).ToBeApproximately(2.0f, 1e-6f);
}

static void Vector3Packet_BatchReject(ITestContext& ctx) noexcept {
    auto v = Vector3Packet<1>{ 
        Packet<float,1>{1.0f}, Packet<float,1>{1.0f}, Packet<float,1>{0.0f} 
    };
    auto n = Vector3Packet<1>{ 
        Packet<float,1>{1.0f}, Packet<float,1>{0.0f}, Packet<float,1>{0.0f} 
    };
    auto rej = Vector3Packet<1>::Reject(v, n);
    That(rej.xs.reg, ctx).ToBeApproximately(0.0f, 1e-6f);
    That(rej.ys.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
}

static void Vector3Packet_BatchReflect(ITestContext& ctx) noexcept {
    auto v = Vector3Packet<1>{ 
        Packet<float,1>{1.0f}, Packet<float,1>{-1.0f}, Packet<float,1>{0.0f} 
    };
    auto n = Vector3Packet<1>{ 
        Packet<float,1>{0.0f}, Packet<float,1>{1.0f}, Packet<float,1>{0.0f} 
    };
    auto reflect = Vector3Packet<1>::Reflect(v, n);
    That(reflect.xs.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
    That(reflect.ys.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
}

static void Vector3Packet_BatchClampLength(ITestContext& ctx) noexcept {
    auto v = Vector3Packet<1>{ 
        Packet<float,1>{10.0f}, Packet<float,1>{0.0f}, Packet<float,1>{0.0f} 
    };
    auto maxLen = Set1<1>(5.0f);
    auto clamped = Vector3Packet<1>::ClampLength(v, maxLen);
    auto clampedLen = Vector3Packet<1>::Length(clamped);
    That(clampedLen.reg, ctx).ToBeApproximately(5.0f, 1e-4f);
}

// ==================== 性能对比基准(简化验证) ====================

static void Benchmark_ScalarVsPacket_Vector3Add(ITestContext& ctx) noexcept {
    constexpr USize kCount = 128;
    SoAChunkVector3<kCount> a{}, b{}, result{};
    
    for (USize i = 0; i < kCount; ++i) {
        a.xs[i] = static_cast<float>(i);
        a.ys[i] = static_cast<float>(i + kCount);
        a.zs[i] = static_cast<float>(i + 2 * kCount);
        b.xs[i] = 1.0f;
        b.ys[i] = 2.0f;
        b.zs[i] = 3.0f;
    }
    
    for (USize i = 0; i < kCount; i += 8) {
        auto pa = LoadPacketAligned<8>(a, i);
        auto pb = LoadPacketAligned<8>(b, i);
        auto pr = Vector3Packet<8>::Add(pa, pb);
        StorePacketAligned(result, i, pr);
    }
    
    for (USize i = 0; i < kCount; ++i) {
        That(result.xs[i], ctx).ToBeApproximately(static_cast<float>(i) + 1.0f, 1e-6f);
        That(result.ys[i], ctx).ToBeApproximately(static_cast<float>(i + kCount) + 2.0f, 1e-6f);
    }
}

static void Benchmark_BatchNormalize_Stability(ITestContext& ctx) noexcept {
    SoAChunkVector3<32> vectors{};
    for (USize i = 0; i < 16; ++i) {
        float scale = static_cast<float>(i + 1);
        vectors.xs[i] = scale;
        vectors.ys[i] = scale * 2.0f;
        vectors.zs[i] = scale * 3.0f;
    }
    
    SoAChunkVector3<32> normalized{};
    for (USize i = 0; i < 16; i += 4) {
        auto v = LoadPacketUnaligned<4>(vectors, i);
        auto n = Vector3Packet<4>::Normalize(v);
        StorePacketUnaligned(normalized, i, n);
    }
    
    for (USize i = 0; i < 16; ++i) {
        float lenSq = normalized.xs[i] * normalized.xs[i] 
                    + normalized.ys[i] * normalized.ys[i] 
                    + normalized.zs[i] * normalized.zs[i];
        That(lenSq, ctx).ToBeApproximately(1.0f, 1e-4f);
    }
}

export int main() {
    Register("Math.Extreme", "Vector3_ExtremeValues_MinMax", Vector3_ExtremeValues_MinMax, false);
    Register("Math.Extreme", "Vector3_NearZero_Normalize", Vector3_NearZero_Normalize, false);
    Register("Math.Extreme", "Quaternion_Identity_Invariance", Quaternion_Identity_Invariance, false);
    Register("Math.Extreme", "Quaternion_Slerp_SameQuaternions", Quaternion_Slerp_SameQuaternions, false);
    Register("Math.Extreme", "Matrix_Compose_UnitScale_Identity", Matrix_Compose_UnitScale_Identity, false);
    
    Register("Math.Stability", "LWC_Precision_LargeWorldCoordinates", LWC_Precision_LargeWorldCoordinates, false);
    Register("Math.Stability", "Half_Precision_Boundary_Values", Half_Precision_Boundary_Values, false);
    
    Register("Math.Batch", "Vector3Packet_BatchProjection", Vector3Packet_BatchProjection, false);
    Register("Math.Batch", "Vector3Packet_BatchReject", Vector3Packet_BatchReject, false);
    Register("Math.Batch", "Vector3Packet_BatchReflect", Vector3Packet_BatchReflect, false);
    Register("Math.Batch", "Vector3Packet_BatchClampLength", Vector3Packet_BatchClampLength, false);
    
    Register("Math.Benchmark", "ScalarVsPacket_Vector3Add", Benchmark_ScalarVsPacket_Vector3Add, false);
    Register("Math.Benchmark", "BatchNormalize_Stability", Benchmark_BatchNormalize_Stability, false);

    RunConfig config{};
    config.workerCount = 4;
    config.frameArenaSize = 128u << 10u;
    config.logLevel = LogLevel::Info;

    auto summary = RunRegistered(config);
    return summary.exitCode;
}
