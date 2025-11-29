module;
export module SIMD:Ops;
import Lang;
import <immintrin.h>;
import :Packet;

export namespace SIMD {
    inline Packet<float, 8> FMA(Packet<float, 8> a, Packet<float, 8> b, Packet<float, 8> c) noexcept {
    #if defined(__FMA__)
        return { _mm256_fmadd_ps(a.reg, b.reg, c.reg) };
    #else
        return { _mm256_add_ps(_mm256_mul_ps(a.reg, b.reg), c.reg) };
    #endif
    }
    inline Packet<float, 4> FMA(Packet<float, 4> a, Packet<float, 4> b, Packet<float, 4> c) noexcept {
    #if defined(__FMA__)
        return { _mm_fmadd_ps(a.reg, b.reg, c.reg) };
    #else
        return { _mm_add_ps(_mm_mul_ps(a.reg, b.reg), c.reg) };
    #endif
    }
    inline Packet<float, 2> FMA(Packet<float, 2> a, Packet<float, 2> b, Packet<float, 2> c) noexcept {
    #if defined(__FMA__)
        return { _mm_fmadd_ps(a.reg, b.reg, c.reg) };
    #else
        return { _mm_add_ps(_mm_mul_ps(a.reg, b.reg), c.reg) };
    #endif
    }
    inline Packet<float, 1> FMA(Packet<float, 1> a, Packet<float, 1> b, Packet<float, 1> c) noexcept { return { a.reg * b.reg + c.reg }; }

    inline Packet<double, 4> FMA(Packet<double, 4> a, Packet<double, 4> b, Packet<double, 4> c) noexcept {
    #if defined(__FMA__)
        return { _mm256_fmadd_pd(a.reg, b.reg, c.reg) };
    #else
        return { _mm256_add_pd(_mm256_mul_pd(a.reg, b.reg), c.reg) };
    #endif
    }
    inline Packet<double, 2> FMA(Packet<double, 2> a, Packet<double, 2> b, Packet<double, 2> c) noexcept {
    #if defined(__FMA__)
        return { _mm_fmadd_pd(a.reg, b.reg, c.reg) };
    #else
        return { _mm_add_pd(_mm_mul_pd(a.reg, b.reg), c.reg) };
    #endif
    }
    inline Packet<double, 1> FMA(Packet<double, 1> a, Packet<double, 1> b, Packet<double, 1> c) noexcept { return { a.reg * b.reg + c.reg }; }

    inline Packet<float, 8> Rsqrt(Packet<float, 8> a) noexcept { return { _mm256_rsqrt_ps(a.reg) }; }
    inline Packet<float, 4> Rsqrt(Packet<float, 4> a) noexcept { return { _mm_rsqrt_ps(a.reg) }; }
    inline Packet<float, 1> Rsqrt(Packet<float, 1> a) noexcept {
        UInt32 i = BitCast<UInt32>(a.reg);
        i = 0x5f3759dfu - (i >> 1);
        float y = BitCast<float>(i);
        return { y };
    }

    template<int W>
    inline Packet<float, W> Recp(Packet<float, W> a) noexcept {
        if constexpr (W == 8) {
            return { _mm256_rcp_ps(a.reg) };
        } else if constexpr (W == 4) {
            return { _mm_rcp_ps(a.reg) };
        } else if constexpr (W == 2) {
            return { _mm_rcp_ps(a.reg) };
        } else {
            return { 1.0f / a.reg };
        }
    }

    template<int W>
    inline Packet<float, W> Set1(float v) noexcept {
        if constexpr (W == 8) { return { _mm256_set1_ps(v) }; }
        else if constexpr (W == 4) { return { _mm_set1_ps(v) }; }
        else if constexpr (W == 2) { return { _mm_set1_ps(v) }; }
        else { return { v }; }
    }
    template<int W>
    inline Packet<double, W> Set1(double v) noexcept {
        if constexpr (W == 4) { return { _mm256_set1_pd(v) }; }
        else if constexpr (W == 2) { return { _mm_set1_pd(v) }; }
        else { return { v }; }
    }

    inline Packet<float, 8> Min(Packet<float, 8> a, Packet<float, 8> b) noexcept { return { _mm256_min_ps(a.reg, b.reg) }; }
    inline Packet<float, 8> Max(Packet<float, 8> a, Packet<float, 8> b) noexcept { return { _mm256_max_ps(a.reg, b.reg) }; }
    inline Packet<float, 8> Abs(Packet<float, 8> a) noexcept { return { _mm256_andnot_ps(_mm256_set1_ps(-0.0f), a.reg) }; }
    inline Packet<float, 4> Min(Packet<float, 4> a, Packet<float, 4> b) noexcept { return { _mm_min_ps(a.reg, b.reg) }; }
    inline Packet<float, 4> Max(Packet<float, 4> a, Packet<float, 4> b) noexcept { return { _mm_max_ps(a.reg, b.reg) }; }
    inline Packet<float, 4> Abs(Packet<float, 4> a) noexcept { return { _mm_andnot_ps(_mm_set1_ps(-0.0f), a.reg) }; }
    inline Packet<float, 1> Min(Packet<float, 1> a, Packet<float, 1> b) noexcept { return { a.reg < b.reg ? a.reg : b.reg }; }
    inline Packet<float, 1> Max(Packet<float, 1> a, Packet<float, 1> b) noexcept { return { a.reg > b.reg ? a.reg : b.reg }; }
    inline Packet<float, 1> Abs(Packet<float, 1> a) noexcept { return { a.reg < 0.0f ? -a.reg : a.reg }; }

    inline Packet<float, 8> And(Packet<float, 8> a, Packet<float, 8> b) noexcept { return { _mm256_and_ps(a.reg, b.reg) }; }
    inline Packet<float, 8> Or(Packet<float, 8> a, Packet<float, 8> b) noexcept { return { _mm256_or_ps(a.reg, b.reg) }; }
    inline Packet<float, 8> Xor(Packet<float, 8> a, Packet<float, 8> b) noexcept { return { _mm256_xor_ps(a.reg, b.reg) }; }
    inline Packet<float, 8> Not(Packet<float, 8> a) noexcept { return { _mm256_xor_ps(a.reg, _mm256_castsi256_ps(_mm256_set1_epi32(-1)) )}; }

    inline Packet<float, 4> And(Packet<float, 4> a, Packet<float, 4> b) noexcept { return { _mm_and_ps(a.reg, b.reg) }; }
    inline Packet<float, 4> Or(Packet<float, 4> a, Packet<float, 4> b) noexcept { return { _mm_or_ps(a.reg, b.reg) }; }
    inline Packet<float, 4> Xor(Packet<float, 4> a, Packet<float, 4> b) noexcept { return { _mm_xor_ps(a.reg, b.reg) }; }
    inline Packet<float, 4> Not(Packet<float, 4> a) noexcept { return { _mm_xor_ps(a.reg, _mm_castsi128_ps(_mm_set1_epi32(-1)) )}; }

    inline Packet<float, 2> And(Packet<float, 2> a, Packet<float, 2> b) noexcept { return { _mm_and_ps(a.reg, b.reg) }; }
    inline Packet<float, 2> Or(Packet<float, 2> a, Packet<float, 2> b) noexcept { return { _mm_or_ps(a.reg, b.reg) }; }
    inline Packet<float, 2> Xor(Packet<float, 2> a, Packet<float, 2> b) noexcept { return { _mm_xor_ps(a.reg, b.reg) }; }
    inline Packet<float, 2> Not(Packet<float, 2> a) noexcept { return { _mm_xor_ps(a.reg, _mm_castsi128_ps(_mm_set1_epi32(-1)) )}; }

    inline Packet<float, 1> And(Packet<float, 1> a, Packet<float, 1> b) noexcept { UInt32 ua = BitCast<UInt32>(a.reg); UInt32 ub = BitCast<UInt32>(b.reg); return { BitCast<float>(ua & ub) }; }
    inline Packet<float, 1> Or(Packet<float, 1> a, Packet<float, 1> b) noexcept { UInt32 ua = BitCast<UInt32>(a.reg); UInt32 ub = BitCast<UInt32>(b.reg); return { BitCast<float>(ua | ub) }; }
    inline Packet<float, 1> Xor(Packet<float, 1> a, Packet<float, 1> b) noexcept { UInt32 ua = BitCast<UInt32>(a.reg); UInt32 ub = BitCast<UInt32>(b.reg); return { BitCast<float>(ua ^ ub) }; }
    inline Packet<float, 1> Not(Packet<float, 1> a) noexcept { UInt32 ua = BitCast<UInt32>(a.reg); return { BitCast<float>(~ua) }; }

    template<int i0, int i1, int i2, int i3>
    inline Packet<float, 8> Swizzle(Packet<float, 8> a) noexcept {
        constexpr int mask = ((i3 & 3) << 6) | ((i2 & 3) << 4) | ((i1 & 3) << 2) | (i0 & 3);
        return { _mm256_permute_ps(a.reg, mask) };
    }
    template<int i0, int i1, int i2, int i3>
    inline Packet<float, 4> Swizzle(Packet<float, 4> a) noexcept {
        return { _mm_shuffle_ps(a.reg, a.reg, _MM_SHUFFLE(i3 & 3, i2 & 3, i1 & 3, i0 & 3)) };
    }
    template<int i0, int i1, int i2, int i3>
    inline Packet<float, 2> Swizzle(Packet<float, 2> a) noexcept { return a; }
    template<int i0, int i1, int i2, int i3>
    inline Packet<float, 1> Swizzle(Packet<float, 1> a) noexcept { return a; }

    inline Packet<float, 8> Permute(Packet<float, 8> a, __m256i idx) noexcept {
    #if defined(__AVX2__)
        return { _mm256_permutevar8x32_ps(a.reg, idx) };
    #else
        return a;
    #endif
    }

    template<int W>
    inline Packet<float, 1> ReduceAdd(Packet<float, W> a) noexcept {
        if constexpr (W == 8) {
            alignas(32) float temp[8]; _mm256_store_ps(temp, a.reg);
            float s = temp[0]+temp[1]+temp[2]+temp[3]+temp[4]+temp[5]+temp[6]+temp[7];
            return { s };
        } else if constexpr (W == 4) {
            alignas(16) float temp[4]; _mm_store_ps(temp, a.reg);
            float s = temp[0]+temp[1]+temp[2]+temp[3];
            return { s };
        } else if constexpr (W == 2) {
            alignas(16) float temp[4]; _mm_store_ps(temp, a.reg);
            float s = temp[0]+temp[1];
            return { s };
        } else {
            return { a.reg };
        }
    }

    template<int W>
    inline Packet<float, 1> ReduceMin(Packet<float, W> a) noexcept {
        if constexpr (W == 8) {
            alignas(32) float temp[8]; _mm256_store_ps(temp, a.reg);
            float m = temp[0]; for (int i=1;i<8;++i) m = m < temp[i] ? m : temp[i];
            return { m };
        } else if constexpr (W == 4) {
            alignas(16) float temp[4]; _mm_store_ps(temp, a.reg);
            float m = temp[0]; for (int i=1;i<4;++i) m = m < temp[i] ? m : temp[i];
            return { m };
        } else if constexpr (W == 2) {
            alignas(16) float temp[4]; _mm_store_ps(temp, a.reg);
            float m = temp[0] < temp[1] ? temp[0] : temp[1];
            return { m };
        } else {
            return { a.reg };
        }
    }

    template<int W>
    inline Packet<float, 1> ReduceMax(Packet<float, W> a) noexcept {
        if constexpr (W == 8) {
            alignas(32) float temp[8]; _mm256_store_ps(temp, a.reg);
            float m = temp[0]; for (int i=1;i<8;++i) m = m > temp[i] ? m : temp[i];
            return { m };
        } else if constexpr (W == 4) {
            alignas(16) float temp[4]; _mm_store_ps(temp, a.reg);
            float m = temp[0]; for (int i=1;i<4;++i) m = m > temp[i] ? m : temp[i];
            return { m };
        } else if constexpr (W == 2) {
            alignas(16) float temp[4]; _mm_store_ps(temp, a.reg);
            float m = temp[0] > temp[1] ? temp[0] : temp[1];
            return { m };
        } else {
            return { a.reg };
        }
    }

    template<int I>
    inline float Extract(Packet<float, 8> a) noexcept {
        alignas(32) float temp[8]; _mm256_store_ps(temp, a.reg);
        return temp[I & 7];
    }
    template<int I>
    inline float Extract(Packet<float, 4> a) noexcept {
        alignas(16) float temp[4]; _mm_store_ps(temp, a.reg);
        return temp[I & 3];
    }
    template<int I>
    inline float Extract(Packet<float, 2> a) noexcept {
        alignas(16) float temp[4]; _mm_store_ps(temp, a.reg);
        return temp[I & 1];
    }
    template<int I>
    inline float Extract(Packet<float, 1> a) noexcept { return a.reg; }

    inline Packet<float, 4> ConvertDoubleToFloat(Packet<double, 4> d) noexcept { return { _mm256_cvtpd_ps(d.reg) }; }
    inline Packet<float, 2> ConvertDoubleToFloat(Packet<double, 2> d) noexcept { return { _mm_cvtpd_ps(d.reg) }; }
    inline Packet<float, 1> ConvertDoubleToFloat(Packet<double, 1> d) noexcept { return { static_cast<float>(d.reg) }; }
}
