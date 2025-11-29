export module Cap.Math:Storage;
import Lang;
import <type_traits>;
export namespace Cap {
    using Scalar = Float32;
    using Real   = Float64;
    using HalfT  = Half;

    struct Vector3 { Scalar x, y, z; };
    struct DVector3 { Real x, y, z; };
    struct Quaternion { Scalar x, y, z, w; };
    struct Matrix4x4 { Scalar m[16]; };

    static_assert(std::is_standard_layout_v<Vector3> && std::is_trivially_copyable_v<Vector3>);
    static_assert(std::is_standard_layout_v<DVector3> && std::is_trivially_copyable_v<DVector3>);
    static_assert(std::is_standard_layout_v<Quaternion> && std::is_trivially_copyable_v<Quaternion>);
    static_assert(std::is_standard_layout_v<Matrix4x4> && std::is_trivially_copyable_v<Matrix4x4>);
    static_assert(sizeof(Vector3) == sizeof(Scalar) * 3);
    static_assert(sizeof(DVector3) == sizeof(Real) * 3);
    static_assert(sizeof(Quaternion) == sizeof(Scalar) * 4);
    static_assert(sizeof(Matrix4x4) == sizeof(Scalar) * 16);

    [[nodiscard]] Vector3 ToFloat(const DVector3& d) noexcept {
        return Vector3{ static_cast<Scalar>(d.x), static_cast<Scalar>(d.y), static_cast<Scalar>(d.z) };
    }
    [[nodiscard]] Scalar HalfToScalar(HalfT h) noexcept {
        UInt16 hb = h.bits;
        UInt32 s = static_cast<UInt32>(hb & 0x8000u) << 16;
        UInt32 e = static_cast<UInt32>(hb & 0x7C00u);
        UInt32 m = static_cast<UInt32>(hb & 0x03FFu);
        UInt32 fb;
        if (e == 0) {
            if (m == 0) {
                fb = s;
            } else {
                UInt32 mm = m;
                int exp = -1;
                while ((mm & 0x0400u) == 0u) { mm <<= 1; --exp; }
                mm &= 0x03FFu;
                UInt32 fe = static_cast<UInt32>((127 - 15 + exp) << 23);
                fb = s | fe | (mm << 13);
            }
        } else if (e == 0x7C00u) {
            fb = s | 0x7F800000u | (m << 13);
        } else {
            UInt32 fe = static_cast<UInt32>(((e >> 10) - 15 + 127) << 23);
            fb = s | fe | (m << 13);
        }
        union { UInt32 u; Scalar f; } cvt{ fb };
        return cvt.f;
    }
    [[nodiscard]] HalfT ScalarToHalf(Scalar f) noexcept {
        union { Scalar f; UInt32 u; } cvt{ f };
        UInt32 u = cvt.u;
        UInt32 s = (u >> 16) & 0x8000u;
        Int32 e = static_cast<Int32>((u >> 23) & 0xFFu) - 127 + 15;
        UInt32 m = u & 0x007FFFFFu;
        UInt16 h;
        if (e <= 0) {
            if (e < -10) {
                h = static_cast<UInt16>(s);
            } else {
                m |= 0x00800000u;
                UInt32 t = m >> static_cast<UInt32>(1 - e);
                UInt32 r = t + 0x00001000u;
                h = static_cast<UInt16>(s | (r >> 13));
            }
        } else if (e >= 31) {
            h = static_cast<UInt16>(s | 0x7C00u);
        } else {
            UInt32 r = m + 0x00001000u;
            h = static_cast<UInt16>(s | (static_cast<UInt32>(e) << 10) | (r >> 13));
        }
        return HalfT{ h };
    }

    template<USize BlockSize = static_cast<USize>(1024)>
    struct SoAChunkVector3 {
        static_assert(BlockSize % static_cast<USize>(16) == 0, "SoAChunkVector3 BlockSize must align to 16 elements");
        alignas(64) Scalar xs[BlockSize];
        alignas(64) Scalar ys[BlockSize];
        alignas(64) Scalar zs[BlockSize];
    };

    template<USize DBlockSize = static_cast<USize>(1024)>
    struct SoAChunkDVector3 {
        static_assert(DBlockSize % static_cast<USize>(16) == 0, "SoAChunkDVector3 BlockSize must align to 16 elements");
        alignas(64) Real xs[DBlockSize];
        alignas(64) Real ys[DBlockSize];
        alignas(64) Real zs[DBlockSize];
    };

    template<USize QBlockSize = static_cast<USize>(1024)>
    struct SoAChunkQuaternion {
        static_assert(QBlockSize % static_cast<USize>(16) == 0, "SoAChunkQuaternion BlockSize must align to 16 elements");
        alignas(64) Scalar xs[QBlockSize];
        alignas(64) Scalar ys[QBlockSize];
        alignas(64) Scalar zs[QBlockSize];
        alignas(64) Scalar ws[QBlockSize];
    };

    template<USize MBlockSize = static_cast<USize>(1024)>
    struct SoAChunkMatrix4x4 {
        static_assert(MBlockSize % static_cast<USize>(16) == 0, "SoAChunkMatrix4x4 BlockSize must align to 16 elements");
        alignas(64) Scalar c0x[MBlockSize];
        alignas(64) Scalar c0y[MBlockSize];
        alignas(64) Scalar c0z[MBlockSize];
        alignas(64) Scalar c0w[MBlockSize];
        alignas(64) Scalar c1x[MBlockSize];
        alignas(64) Scalar c1y[MBlockSize];
        alignas(64) Scalar c1z[MBlockSize];
        alignas(64) Scalar c1w[MBlockSize];
        alignas(64) Scalar c2x[MBlockSize];
        alignas(64) Scalar c2y[MBlockSize];
        alignas(64) Scalar c2z[MBlockSize];
        alignas(64) Scalar c2w[MBlockSize];
        alignas(64) Scalar c3x[MBlockSize];
        alignas(64) Scalar c3y[MBlockSize];
        alignas(64) Scalar c3z[MBlockSize];
        alignas(64) Scalar c3w[MBlockSize];
    };

    template<USize BlockSize>
    inline constexpr bool kIsSoAChunkVector3Aligned = alignof(SoAChunkVector3<BlockSize>) == 64;
    template<USize BlockSize>
    inline constexpr bool kIsSoAChunkDVector3Aligned = alignof(SoAChunkDVector3<BlockSize>) == 64;
    template<USize BlockSize>
    inline constexpr bool kIsSoAChunkQuaternionAligned = alignof(SoAChunkQuaternion<BlockSize>) == 64;
    template<USize BlockSize>
    inline constexpr bool kIsSoAChunkMatrix4x4Aligned = alignof(SoAChunkMatrix4x4<BlockSize>) == 64;

    static_assert(kIsSoAChunkVector3Aligned<16>);
    static_assert(kIsSoAChunkDVector3Aligned<16>);
    static_assert(kIsSoAChunkQuaternionAligned<16>);
    static_assert(kIsSoAChunkMatrix4x4Aligned<16>);
}
