module;
export module Cap.Math:Random;
import Lang;
import SIMD;
import :Storage;
import :Transcendental;
import <cmath>;
export namespace Cap {
    template<int W>
    [[nodiscard]] inline SIMD::Packet<float,W> Uniform(SIMD::Packet<float,W> seed) noexcept {
        auto a = SIMD::Set1<W>(12.9898f);
        auto b = SIMD::Set1<W>(78.233f);
        auto s = Transcendental::Sin<W>(a * seed + b);
        return (s + SIMD::Set1<W>(1.0f)) * SIMD::Set1<W>(0.5f);
    }
    template<int W>
    [[nodiscard]] inline SIMD::Packet<float,W> Gaussian(SIMD::Packet<float,W> seed) noexcept {
        auto s0 = seed;
        auto s1 = seed + SIMD::Set1<W>(1.0f);
        auto s2 = seed + SIMD::Set1<W>(2.0f);
        auto s3 = seed + SIMD::Set1<W>(3.0f);
        auto s4 = seed + SIMD::Set1<W>(4.0f);
        auto s5 = seed + SIMD::Set1<W>(5.0f);
        auto s6 = seed + SIMD::Set1<W>(6.0f);
        auto s7 = seed + SIMD::Set1<W>(7.0f);
        auto s8 = seed + SIMD::Set1<W>(8.0f);
        auto s9 = seed + SIMD::Set1<W>(9.0f);
        auto s10 = seed + SIMD::Set1<W>(10.0f);
        auto s11 = seed + SIMD::Set1<W>(11.0f);
        auto u = Uniform<W>(s0) + Uniform<W>(s1) + Uniform<W>(s2) + Uniform<W>(s3) + Uniform<W>(s4) + Uniform<W>(s5) + Uniform<W>(s6) + Uniform<W>(s7) + Uniform<W>(s8) + Uniform<W>(s9) + Uniform<W>(s10) + Uniform<W>(s11);
        return u - SIMD::Set1<W>(6.0f);
    }
    [[nodiscard]] inline Scalar UniformScalar(UInt32 seed) noexcept {
        Scalar x = std::sin(seed * 12.9898f + 78.233f) * 43758.5453f;
        return x - std::floor(x);
    }
    [[nodiscard]] inline Scalar Noise3(const Vector3& p) noexcept {
        auto fade = [](Scalar t) noexcept -> Scalar { return t*t*(3.0f - 2.0f*t); };
        auto hash = [](int x, int y, int z) noexcept -> Scalar {
            UInt32 h = static_cast<UInt32>(x) * 374761393u + static_cast<UInt32>(y) * 668265263u + static_cast<UInt32>(z) * 2147483647u;
            h = (h ^ (h >> 13)) * 1274126177u;
            return (h & 0xFFFFFFu) / 16777215.0f;
        };
        int X = static_cast<int>(std::floor(p.x));
        int Y = static_cast<int>(std::floor(p.y));
        int Z = static_cast<int>(std::floor(p.z));
        Scalar fx = p.x - X;
        Scalar fy = p.y - Y;
        Scalar fz = p.z - Z;
        Scalar u = fade(fx), v = fade(fy), w = fade(fz);
        Scalar c000 = hash(X, Y, Z);
        Scalar c100 = hash(X+1, Y, Z);
        Scalar c010 = hash(X, Y+1, Z);
        Scalar c110 = hash(X+1, Y+1, Z);
        Scalar c001 = hash(X, Y, Z+1);
        Scalar c101 = hash(X+1, Y, Z+1);
        Scalar c011 = hash(X, Y+1, Z+1);
        Scalar c111 = hash(X+1, Y+1, Z+1);
        Scalar x00 = c000*(1.0f-u) + c100*u;
        Scalar x10 = c010*(1.0f-u) + c110*u;
        Scalar x01 = c001*(1.0f-u) + c101*u;
        Scalar x11 = c011*(1.0f-u) + c111*u;
        Scalar y0 = x00*(1.0f-v) + x10*v;
        Scalar y1 = x01*(1.0f-v) + x11*v;
        return y0*(1.0f-w) + y1*w;
    }
}
