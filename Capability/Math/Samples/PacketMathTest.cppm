module;

import <cmath>;
import <cstdint>;
import <limits>;

export module PacketMathTest;

import Language;
import Test;
import Math;
import SIMD;

using namespace Math;
using namespace SIMD;
using namespace Test;

static void Vector3_Dot_Cross_And_Length(ITestContext& ctx) noexcept {
    auto a1 = Vector3Packet<1>{ Packet<float,1>{1.0f}, Packet<float,1>{0.0f}, Packet<float,1>{0.0f} };
    auto b1 = Vector3Packet<1>{ Packet<float,1>{0.0f}, Packet<float,1>{1.0f}, Packet<float,1>{0.0f} };
    auto d1 = Vector3Packet<1>::Dot(a1, b1);
    That(d1.reg, ctx).ToBeApproximately(0.0f, 1e-6f);
    auto c1 = Vector3Packet<1>::Cross(a1, b1);
    That(c1.xs.reg, ctx).ToBeApproximately(0.0f, 1e-6f);
    That(c1.ys.reg, ctx).ToBeApproximately(0.0f, 1e-6f);
    That(c1.zs.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
    auto l1 = Vector3Packet<1>::Length(a1);
    That(l1.reg, ctx).ToBeApproximately(1.0f, 1e-6f);

    float ax4[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    float ay4[4] = {0.0f, 1.0f, 0.0f, 0.0f};
    float az4[4] = {0.0f, 0.0f, 1.0f, 0.0f};
    float bx4[4] = {0.0f, 1.0f, 0.0f, 0.0f};
    float by4[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    float bz4[4] = {0.0f, 0.0f, 1.0f, 0.0f};
    auto a4 = Vector3Packet<4>{ Packet<float,4>::LoadUnaligned(ax4), Packet<float,4>::LoadUnaligned(ay4), Packet<float,4>::LoadUnaligned(az4) };
    auto b4 = Vector3Packet<4>{ Packet<float,4>::LoadUnaligned(bx4), Packet<float,4>::LoadUnaligned(by4), Packet<float,4>::LoadUnaligned(bz4) };
    auto d4 = Vector3Packet<4>::Dot(a4, b4);
    float d4v[4]; Packet<float,4>::StoreUnaligned(d4v, d4);
    That(d4v[0], ctx).ToBeApproximately(0.0f, 1e-6f);
    That(d4v[1], ctx).ToBeApproximately(0.0f, 1e-6f);
    That(d4v[2], ctx).ToBeApproximately(1.0f, 1e-6f);
    That(d4v[3], ctx).ToBeApproximately(0.0f, 1e-6f);

    float ax8[8] = {1,0,0,0, 0,1,0,0};
    float ay8[8] = {0,1,0,0, 0,0,1,0};
    float az8[8] = {0,0,1,0, 0,0,0,1};
    float bx8[8] = {0,1,0,0, 1,0,0,0};
    float by8[8] = {1,0,0,0, 0,1,0,0};
    float bz8[8] = {0,0,1,0, 0,0,1,0};
    auto a8 = Vector3Packet<8>{ Packet<float,8>::LoadUnaligned(ax8), Packet<float,8>::LoadUnaligned(ay8), Packet<float,8>::LoadUnaligned(az8) };
    auto b8 = Vector3Packet<8>{ Packet<float,8>::LoadUnaligned(bx8), Packet<float,8>::LoadUnaligned(by8), Packet<float,8>::LoadUnaligned(bz8) };
    auto d8 = Vector3Packet<8>::Dot(a8, b8);
    float d8v[8]; Packet<float,8>::StoreUnaligned(d8v, d8);
    That(d8v[0], ctx).ToBeApproximately(0.0f, 1e-6f);
    That(d8v[1], ctx).ToBeApproximately(0.0f, 1e-6f);
    That(d8v[2], ctx).ToBeApproximately(1.0f, 1e-6f);
    That(d8v[3], ctx).ToBeApproximately(0.0f, 1e-6f);
}

static void Vector3_Normalize_Zero_Safe(ITestContext& ctx) noexcept {
    auto z = Vector3Packet<1>{ Packet<float,1>{0.0f}, Packet<float,1>{0.0f}, Packet<float,1>{0.0f} };
    auto n = Vector3Packet<1>::Normalize(z);
    That(n.xs.reg, ctx).ToBeApproximately(0.0f, 1e-6f);
    That(n.ys.reg, ctx).ToBeApproximately(0.0f, 1e-6f);
    That(n.zs.reg, ctx).ToBeApproximately(0.0f, 1e-6f);
}

static void Quaternion_Rotate_Z90(ITestContext& ctx) noexcept {
    float s = 0.70710678f;
    auto q = QuaternionPacket<1>{ Packet<float,1>{0.0f}, Packet<float,1>{0.0f}, Packet<float,1>{s}, Packet<float,1>{s} };
    auto v = Vector3Packet<1>{ Packet<float,1>{1.0f}, Packet<float,1>{0.0f}, Packet<float,1>{0.0f} };
    auto r = QuaternionPacket<1>::RotateVector(q, v);
    That(r.xs.reg, ctx).ToBeApproximately(0.0f, 1e-5f);
    That(r.ys.reg, ctx).ToBeApproximately(1.0f, 1e-5f);
    That(r.zs.reg, ctx).ToBeApproximately(0.0f, 1e-6f);
}

static void Matrix_Compose_TRS_Z90(ITestContext& ctx) noexcept {
    float s = 0.70710678f;
    auto q = QuaternionPacket<1>{ Packet<float,1>{0.0f}, Packet<float,1>{0.0f}, Packet<float,1>{s}, Packet<float,1>{s} };
    auto p = Vector3Packet<1>{ Packet<float,1>{1.0f}, Packet<float,1>{2.0f}, Packet<float,1>{3.0f} };
    auto sc = Vector3Packet<1>{ Packet<float,1>{1.0f}, Packet<float,1>{1.0f}, Packet<float,1>{1.0f} };
    auto m = MatrixPacket<1>::Compose(p, q, sc);
    That(m.c0x.reg, ctx).ToBeApproximately(0.0f, 1e-5f);
    That(m.c0y.reg, ctx).ToBeApproximately(1.0f, 1e-5f);
    That(m.c1x.reg, ctx).ToBeApproximately(-1.0f, 1e-5f);
    That(m.c1y.reg, ctx).ToBeApproximately(0.0f, 1e-5f);
    That(m.c2z.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
    That(m.c3w.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
    That(m.c3x.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
    That(m.c3y.reg, ctx).ToBeApproximately(2.0f, 1e-6f);
    That(m.c3z.reg, ctx).ToBeApproximately(3.0f, 1e-6f);
}

static void Rsqrt_Accuracy_Check(ITestContext& ctx) noexcept {
    float xs[4] = {0.5f, 1.0f, 4.0f, 100.0f};
    auto p = Packet<float,4>::LoadUnaligned(xs);
    auto r = Rsqrt(p);
    float rv[4]; Packet<float,4>::StoreUnaligned(rv, r);
    for (int i = 0; i < 4; ++i) {
        float e = 1.0f / std::sqrt(xs[i]);
        That(rv[i], ctx).ToBeApproximately(e, 5e-2f);
    }
}

static void Storage_HalfConversion_RoundTrip(ITestContext& ctx) noexcept {
    constexpr float samples[] = { -65504.0f, -42.5f, -0.0f, 0.0f, 1.0f, 3.1415926f, 16384.0f };
    for (float v : samples) {
        auto h = ScalarToHalf(v);
        auto roundtrip = HalfToScalar(h);
        That(roundtrip, ctx).ToBeApproximately(v, 1e-3f);
    }
    auto infHalf = ScalarToHalf(std::numeric_limits<float>::infinity());
    auto inf = HalfToScalar(infHalf);
    That(inf, ctx).ToBe(std::numeric_limits<float>::infinity());
}

static void Storage_ToFloat_And_SoAAlignment(ITestContext& ctx) noexcept {
    DVector3 world{ 1.5, -2.125, 42.75 };
    auto local = ToFloat(world);
    That(local.x, ctx).ToBeApproximately(1.5f, 1e-6f);
    That(local.y, ctx).ToBeApproximately(-2.125f, 1e-6f);
    That(local.z, ctx).ToBeApproximately(42.75f, 1e-6f);

    SoAChunkVector3<> vectors{};
    SoAChunkQuaternion<> rotations{};
    SoAChunkMatrix4x4<> matrices{};

    auto checkAlignment = [&](auto* ptr) {
        auto addr = reinterpret_cast<std::uintptr_t>(ptr);
        That(addr % 64u, ctx).ToBe(static_cast<std::uintptr_t>(0));
    };

    checkAlignment(&vectors.xs[0]);
    checkAlignment(&vectors.ys[0]);
    checkAlignment(&vectors.zs[0]);
    checkAlignment(&rotations.xs[0]);
    checkAlignment(&rotations.ws[0]);
    checkAlignment(&matrices.c0x[0]);
    checkAlignment(&matrices.c3w[0]);

    constexpr USize vectorCapacity = sizeof(vectors.xs) / sizeof(Scalar);
    constexpr USize matrixCapacity = sizeof(matrices.c0x) / sizeof(Scalar);
    That(vectorCapacity % static_cast<USize>(16), ctx).ToBe(static_cast<USize>(0));
    That(matrixCapacity % static_cast<USize>(16), ctx).ToBe(static_cast<USize>(0));
}

static void Compute_MaskedLoadStore_TailHandling(ITestContext& ctx) noexcept {
    SoAChunkVector3<32> chunk{};
    for (USize i = 0; i < 10; ++i) {
        chunk.xs[i] = static_cast<float>(i);
        chunk.ys[i] = static_cast<float>(i + 100);
        chunk.zs[i] = static_cast<float>(i + 200);
    }

    auto packet = LoadPacketMaskedUnaligned<8>(chunk, 0, 10);
    SoAChunkVector3<32> out{};
    StorePacketMaskedUnaligned(out, 0, packet, 10);

    for (USize i = 0; i < 10; ++i) {
        That(out.xs[i], ctx).ToBeApproximately(chunk.xs[i], 1e-6f);
        That(out.ys[i], ctx).ToBeApproximately(chunk.ys[i], 1e-6f);
        That(out.zs[i], ctx).ToBeApproximately(chunk.zs[i], 1e-6f);
    }

    SoAChunkQuaternion<32> qChunk{};
    for (USize i = 0; i < 6; ++i) {
        qChunk.xs[i] = static_cast<float>(i);
        qChunk.ys[i] = static_cast<float>(i + 10);
        qChunk.zs[i] = static_cast<float>(i + 20);
        qChunk.ws[i] = static_cast<float>(i + 30);
    }

    auto qPacket = LoadPacketMaskedUnaligned<4>(qChunk, 0, 6);
    SoAChunkQuaternion<32> qOut{};
    StorePacketMaskedUnaligned(qOut, 0, qPacket, 6);

    for (USize i = 0; i < 6; ++i) {
        That(qOut.xs[i], ctx).ToBeApproximately(qChunk.xs[i], 1e-6f);
        That(qOut.ys[i], ctx).ToBeApproximately(qChunk.ys[i], 1e-6f);
        That(qOut.zs[i], ctx).ToBeApproximately(qChunk.zs[i], 1e-6f);
        That(qOut.ws[i], ctx).ToBeApproximately(qChunk.ws[i], 1e-6f);
    }
}

static void Vector3Packet_Lerp_Min_Max_Clamp(ITestContext& ctx) noexcept {
    auto a = Vector3Packet<1>{ Packet<float,1>{1.0f}, Packet<float,1>{2.0f}, Packet<float,1>{3.0f} };
    auto b = Vector3Packet<1>{ Packet<float,1>{5.0f}, Packet<float,1>{6.0f}, Packet<float,1>{7.0f} };
    auto t = Packet<float,1>{0.25f};
    auto lerp = Vector3Packet<1>::Lerp(a, b, t);
    That(lerp.xs.reg, ctx).ToBeApproximately(2.0f, 1e-6f);
    That(lerp.ys.reg, ctx).ToBeApproximately(3.0f, 1e-6f);
    That(lerp.zs.reg, ctx).ToBeApproximately(4.0f, 1e-6f);

    auto minV = Vector3Packet<1>::Min(a, b);
    That(minV.xs.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
    That(minV.ys.reg, ctx).ToBeApproximately(2.0f, 1e-6f);
    That(minV.zs.reg, ctx).ToBeApproximately(3.0f, 1e-6f);

    auto maxV = Vector3Packet<1>::Max(a, b);
    That(maxV.xs.reg, ctx).ToBeApproximately(5.0f, 1e-6f);
    That(maxV.ys.reg, ctx).ToBeApproximately(6.0f, 1e-6f);
    That(maxV.zs.reg, ctx).ToBeApproximately(7.0f, 1e-6f);

    auto test = Vector3Packet<1>{ Packet<float,1>{0.0f}, Packet<float,1>{4.0f}, Packet<float,1>{10.0f} };
    auto clamped = Vector3Packet<1>::Clamp(test, a, b);
    That(clamped.xs.reg, ctx).ToBeApproximately(1.0f, 1e-6f);
    That(clamped.ys.reg, ctx).ToBeApproximately(4.0f, 1e-6f);
    That(clamped.zs.reg, ctx).ToBeApproximately(7.0f, 1e-6f);
}

static void QuaternionPacket_Conjugate_Multiply_Slerp(ITestContext& ctx) noexcept {
    auto q = QuaternionPacket<1>{ Packet<float,1>{0.1f}, Packet<float,1>{0.2f}, Packet<float,1>{0.3f}, Packet<float,1>{0.9274f} };
    auto conj = QuaternionPacket<1>::Conjugate(q);
    That(conj.xs.reg, ctx).ToBeApproximately(-0.1f, 1e-6f);
    That(conj.ys.reg, ctx).ToBeApproximately(-0.2f, 1e-6f);
    That(conj.zs.reg, ctx).ToBeApproximately(-0.3f, 1e-6f);
    That(conj.ws.reg, ctx).ToBeApproximately(0.9274f, 1e-4f);

    auto identity = QuaternionPacket<1>{ Packet<float,1>{0.0f}, Packet<float,1>{0.0f}, Packet<float,1>{0.0f}, Packet<float,1>{1.0f} };
    auto mul = QuaternionPacket<1>::Multiply(q, identity);
    That(mul.xs.reg, ctx).ToBeApproximately(0.1f, 1e-6f);
    That(mul.ys.reg, ctx).ToBeApproximately(0.2f, 1e-6f);
    That(mul.zs.reg, ctx).ToBeApproximately(0.3f, 1e-6f);
    That(mul.ws.reg, ctx).ToBeApproximately(0.9274f, 1e-4f);

    auto slerp = QuaternionPacket<1>::Slerp(identity, q, Packet<float,1>{0.5f});
    auto norm = QuaternionPacket<1>::Normalize(slerp);
    auto dot = QuaternionPacket<1>::Dot(norm, norm);
    That(dot.reg, ctx).ToBeApproximately(1.0f, 1e-5f);
}

static void LWC_BatchConvertPacket4(ITestContext& ctx) noexcept {
    SoAChunkDVector3<16> worldPos{};
    for (USize i = 0; i < 10; ++i) {
        worldPos.xs[i] = 1000000.0 + static_cast<double>(i);
        worldPos.ys[i] = 2000000.0 + static_cast<double>(i * 2);
        worldPos.zs[i] = 3000000.0 + static_cast<double>(i * 3);
    }

    DVector3 camPos{ 1000000.0, 2000000.0, 3000000.0 };
    SoAChunkVector3<16> localPos{};

    LWCConverter::BatchConvertPacket4(worldPos, camPos, localPos, 10);

    for (USize i = 0; i < 10; ++i) {
        That(localPos.xs[i], ctx).ToBeApproximately(static_cast<float>(i), 1e-4f);
        That(localPos.ys[i], ctx).ToBeApproximately(static_cast<float>(i * 2), 1e-4f);
        That(localPos.zs[i], ctx).ToBeApproximately(static_cast<float>(i * 3), 1e-4f);
    }
}

export int main() {
    Register("Math.Packet", "Vector3_Dot_Cross_And_Length", Vector3_Dot_Cross_And_Length, false);
    Register("Math.Packet", "Vector3_Normalize_Zero_Safe", Vector3_Normalize_Zero_Safe, false);
    Register("Math.Packet", "Quaternion_Rotate_Z90", Quaternion_Rotate_Z90, false);
    Register("Math.Packet", "Matrix_Compose_TRS_Z90", Matrix_Compose_TRS_Z90, false);
    Register("Math.Packet", "Rsqrt_Accuracy_Check", Rsqrt_Accuracy_Check, false);
    Register("Math.Storage", "HalfConversion_RoundTrip", Storage_HalfConversion_RoundTrip, false);
    Register("Math.Storage", "ToFloat_And_SoAAlignment", Storage_ToFloat_And_SoAAlignment, false);
    Register("Math.Compute", "MaskedLoadStore_TailHandling", Compute_MaskedLoadStore_TailHandling, false);
    Register("Math.Compute", "Vector3Packet_Lerp_Min_Max_Clamp", Vector3Packet_Lerp_Min_Max_Clamp, false);
    Register("Math.Compute", "QuaternionPacket_Conjugate_Multiply_Slerp", QuaternionPacket_Conjugate_Multiply_Slerp, false);
    Register("Math.LWC", "BatchConvertPacket4", LWC_BatchConvertPacket4, false);

    RunConfig config{};
    config.workerCount = 4;
    config.frameArenaSize = 128u << 10u;
    config.logLevel = LogLevel::Info;

    auto summary = RunRegistered(config);
    return summary.exitCode;
}
