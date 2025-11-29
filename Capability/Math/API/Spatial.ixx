module;
export module Cap.Math:Spatial;
import Lang;
import :Storage;
import :Compute;
export namespace Cap {
    struct Morton {
        static inline UInt32 Part1By2(UInt32 x) noexcept {
            x &= 0x000003FFu;
            x = (x | (x << 16)) & 0x30000FFu;
            x = (x | (x << 8)) & 0x300F00Fu;
            x = (x | (x << 4)) & 0x30C30C3u;
            x = (x | (x << 2)) & 0x9249249u;
            return x;
        }
        static inline UInt64 Part1By2_64(UInt64 x) noexcept {
            x &= 0x00000000001FFFFFull;
            x = (x | (x << 32)) & 0x001F00000000FFFFull;
            x = (x | (x << 16)) & 0x001F0000FF0000FFull;
            x = (x | (x << 8))  & 0x001F00F00F00F00Full;
            x = (x | (x << 4))  & 0x001F0C30C30C30C3ull;
            x = (x | (x << 2))  & 0x0019249249249249ull;
            return x;
        }
        static inline UInt32 Encode3D(UInt32 x, UInt32 y, UInt32 z) noexcept {
            return Part1By2(x) | (Part1By2(y) << 1) | (Part1By2(z) << 2);
        }
        static inline UInt64 Encode3D_64(UInt32 x, UInt32 y, UInt32 z) noexcept {
            UInt64 X = Part1By2_64(x);
            UInt64 Y = Part1By2_64(y);
            UInt64 Z = Part1By2_64(z);
            return X | (Y << 1) | (Z << 2);
        }
    };
    template<int W>
    [[nodiscard]] inline Vector3Packet<W> BezierQuadratic(Vector3Packet<W> p0, Vector3Packet<W> p1, Vector3Packet<W> p2, SIMD::Packet<float,W> t) noexcept {
        auto one = SIMD::Set1<W>(1.0f);
        auto u = one - t;
        auto uu = u * u;
        auto tt = t * t;
        auto a = Vector3Packet<W>::Mul(p0, uu);
        auto b = Vector3Packet<W>::Mul(p1, u * t * SIMD::Set1<W>(2.0f));
        auto c = Vector3Packet<W>::Mul(p2, tt);
        return Vector3Packet<W>::Add(Vector3Packet<W>::Add(a, b), c);
    }
    template<int W>
    [[nodiscard]] inline Vector3Packet<W> BezierCubic(Vector3Packet<W> p0, Vector3Packet<W> p1, Vector3Packet<W> p2, Vector3Packet<W> p3, SIMD::Packet<float,W> t) noexcept {
        auto one = SIMD::Set1<W>(1.0f);
        auto u = one - t;
        auto uu = u * u;
        auto uuu = uu * u;
        auto tt = t * t;
        auto ttt = tt * t;
        auto a = Vector3Packet<W>::Mul(p0, uuu);
        auto b = Vector3Packet<W>::Mul(p1, SIMD::Set1<W>(3.0f) * uu * t);
        auto c = Vector3Packet<W>::Mul(p2, SIMD::Set1<W>(3.0f) * u * tt);
        auto d = Vector3Packet<W>::Mul(p3, ttt);
        return Vector3Packet<W>::Add(Vector3Packet<W>::Add(a, b), Vector3Packet<W>::Add(c, d));
    }
    template<int W>
    [[nodiscard]] inline Vector3Packet<W> CatmullRom(Vector3Packet<W> p0, Vector3Packet<W> p1, Vector3Packet<W> p2, Vector3Packet<W> p3, SIMD::Packet<float,W> t) noexcept {
        auto t2 = t * t;
        auto t3 = t2 * t;
        auto a = Vector3Packet<W>::Mul(p1, SIMD::Set1<W>(2.0f));
        auto b = Vector3Packet<W>::Mul(Vector3Packet<W>::Sub(p2, p0), t);
        auto c = Vector3Packet<W>::Mul(Vector3Packet<W>::Add(Vector3Packet<W>::Mul(p0, SIMD::Set1<W>(2.0f)), Vector3Packet<W>::Sub(Vector3Packet<W>::Mul(p2, SIMD::Set1<W>(-5.0f)), p1)), t2);
        auto d = Vector3Packet<W>::Mul(Vector3Packet<W>::Add(Vector3Packet<W>::Sub(Vector3Packet<W>::Mul(p3, SIMD::Set1<W>(-1.0f)), p0), Vector3Packet<W>::Mul(p2, SIMD::Set1<W>(3.0f))), t3);
        auto res = Vector3Packet<W>::Add(Vector3Packet<W>::Add(a, b), Vector3Packet<W>::Add(c, d));
        return Vector3Packet<W>::Mul(res, SIMD::Set1<W>(0.5f));
    }
    [[nodiscard]] inline Scalar SmoothStep(Scalar a, Scalar b, Scalar x) noexcept {
        Scalar t = (x - a) / (b - a);
        if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
        return t * t * (3.0f - 2.0f * t);
    }
    [[nodiscard]] inline Scalar SmootherStep(Scalar a, Scalar b, Scalar x) noexcept {
        Scalar t = (x - a) / (b - a);
        if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }
    [[nodiscard]] inline Scalar Step(Scalar edge, Scalar x) noexcept { return x < edge ? 0.0f : 1.0f; }
}

