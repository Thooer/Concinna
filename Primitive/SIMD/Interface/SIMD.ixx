module;
export module SIMD;
import Language;
import System;
import <immintrin.h>;

export namespace SIMD {
    template<typename T, int W>
    struct Packet;

    template<>
    struct Packet<float, 8> {
        __m256 reg;
        inline Packet operator+(Packet rhs) const noexcept { return { _mm256_add_ps(reg, rhs.reg) }; }
        inline Packet operator-(Packet rhs) const noexcept { return { _mm256_sub_ps(reg, rhs.reg) }; }
        inline Packet operator*(Packet rhs) const noexcept { return { _mm256_mul_ps(reg, rhs.reg) }; }
        static inline Packet LoadAligned(const float* p) noexcept { return { _mm256_load_ps(p) }; }
        static inline Packet LoadUnaligned(const float* p) noexcept { return { _mm256_loadu_ps(p) }; }
        static inline void StoreAligned(float* p, Packet v) noexcept { _mm256_store_ps(p, v.reg); }
        static inline void StoreUnaligned(float* p, Packet v) noexcept { _mm256_storeu_ps(p, v.reg); }
    };
    inline Packet<float, 8> FMA(Packet<float, 8> a, Packet<float, 8> b, Packet<float, 8> c) noexcept {
    #if defined(__FMA__)
        return { _mm256_fmadd_ps(a.reg, b.reg, c.reg) };
    #else
        return { _mm256_add_ps(_mm256_mul_ps(a.reg, b.reg), c.reg) };
    #endif
    }
    inline Packet<float, 8> Rsqrt(Packet<float, 8> a) noexcept { return { _mm256_rsqrt_ps(a.reg) }; }
    template<int i0, int i1, int i2, int i3>
    inline Packet<float, 8> Swizzle(Packet<float, 8> a) noexcept {
        constexpr int mask = ((i3 & 3) << 6) | ((i2 & 3) << 4) | ((i1 & 3) << 2) | (i0 & 3);
        return { _mm256_permute_ps(a.reg, mask) };
    }

    template<>
    struct Packet<float, 4> {
        __m128 reg;
        inline Packet operator+(Packet rhs) const noexcept { return { _mm_add_ps(reg, rhs.reg) }; }
        inline Packet operator-(Packet rhs) const noexcept { return { _mm_sub_ps(reg, rhs.reg) }; }
        inline Packet operator*(Packet rhs) const noexcept { return { _mm_mul_ps(reg, rhs.reg) }; }
        static inline Packet LoadAligned(const float* p) noexcept { return { _mm_load_ps(p) }; }
        static inline Packet LoadUnaligned(const float* p) noexcept { return { _mm_loadu_ps(p) }; }
        static inline void StoreAligned(float* p, Packet v) noexcept { _mm_store_ps(p, v.reg); }
        static inline void StoreUnaligned(float* p, Packet v) noexcept { _mm_storeu_ps(p, v.reg); }};

    inline Packet<float, 4> FMA(Packet<float, 4> a, Packet<float, 4> b, Packet<float, 4> c) noexcept {
    #if defined(__FMA__)
        return { _mm_fmadd_ps(a.reg, b.reg, c.reg) };
    #else
        return { _mm_add_ps(_mm_mul_ps(a.reg, b.reg), c.reg) };
    #endif
    }
    inline Packet<float, 4> Rsqrt(Packet<float, 4> a) noexcept { return { _mm_rsqrt_ps(a.reg) }; }
    template<int i0, int i1, int i2, int i3>
    inline Packet<float, 4> Swizzle(Packet<float, 4> a) noexcept {
        return { _mm_shuffle_ps(a.reg, a.reg, _MM_SHUFFLE(i3 & 3, i2 & 3, i1 & 3, i0 & 3)) };
    }

    template<>
    struct Packet<float, 1> {
        float reg;
        inline Packet operator+(Packet rhs) const noexcept { return { reg + rhs.reg };
        }
        inline Packet operator-(Packet rhs) const noexcept { return { reg - rhs.reg };
        }
        inline Packet operator*(Packet rhs) const noexcept { return { reg * rhs.reg };
        }
        static inline Packet LoadAligned(const float* p) noexcept { return { *p }; }
        static inline Packet LoadUnaligned(const float* p) noexcept { return { *p }; }
        static inline void StoreAligned(float* p, Packet v) noexcept { *p = v.reg; }
        static inline void StoreUnaligned(float* p, Packet v) noexcept { *p = v.reg; }};
    inline Packet<float, 1> FMA(Packet<float, 1> a, Packet<float, 1> b, Packet<float, 1> c) noexcept { return { a.reg * b.reg + c.reg }; }
    inline Packet<float, 1> Rsqrt(Packet<float, 1> a) noexcept {
        UInt32 i = Language::BitCast<UInt32>(a.reg);
        i = 0x5f3759dfu - (i >> 1);
        float y = Language::BitCast<float>(i);
        return { y };
    }
    template<int i0, int i1, int i2, int i3>
    inline Packet<float, 1> Swizzle(Packet<float, 1> a) noexcept { return a; }

    template<int W>
    struct PacketMask;
    template<>
    struct PacketMask<8> { __m256 reg; };
    template<>
    struct PacketMask<4> { __m128 reg; };
    template<>
    struct PacketMask<1> { int reg; };

    inline PacketMask<8> GreaterThan(Packet<float, 8> a, Packet<float, 8> b) noexcept { return { _mm256_cmp_ps(a.reg, b.reg, _CMP_GT_OQ) }; }
    inline Packet<float, 8> Select(PacketMask<8> m, Packet<float, 8> t, Packet<float, 8> f) noexcept { return { _mm256_blendv_ps(f.reg, t.reg, m.reg) }; }

    inline PacketMask<4> GreaterThan(Packet<float, 4> a, Packet<float, 4> b) noexcept { return { _mm_cmp_ps(a.reg, b.reg, _CMP_GT_OQ) }; }
    inline Packet<float, 4> Select(PacketMask<4> m, Packet<float, 4> t, Packet<float, 4> f) noexcept { return { _mm_blendv_ps(f.reg, t.reg, m.reg) }; }

    inline PacketMask<1> GreaterThan(Packet<float, 1> a, Packet<float, 1> b) noexcept { return { a.reg > b.reg ? 1 : 0 }; }
    inline Packet<float, 1> Select(PacketMask<1> m, Packet<float, 1> t, Packet<float, 1> f) noexcept { return { m.reg ? t.reg : f.reg }; }
    template<int W>
    inline Packet<float, W> Set1(float v) noexcept {
        if constexpr (W == 8) { return { _mm256_set1_ps(v) }; }
        else if constexpr (W == 4) { return { _mm_set1_ps(v) }; }
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

    inline UInt8 EqMask8(const UInt8* p, UInt8 b) noexcept {
    #if defined(__AVX2__) || defined(__SSE2__) || defined(_M_X64) || defined(_M_IX86)
        __m128i vec = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(p));
        __m128i hb  = _mm_set1_epi8(static_cast<char>(b));
        __m128i cmp = _mm_cmpeq_epi8(vec, hb);
        int m       = _mm_movemask_epi8(cmp);
        return static_cast<UInt8>(m & 0xFF);
    #else
        UInt64 word = *reinterpret_cast<const UInt64*>(p);
        UInt64 rpt = static_cast<UInt64>(b) * 0x0101010101010101ull;
        UInt64 x = word ^ rpt;
        UInt64 t = (x - 0x0101010101010101ull) & (~x) & 0x8080808080808080ull;
        UInt64 z = (t >> 7) & 0x0101010101010101ull;
        UInt64 packed = (z * 0x0102040810204080ull) >> 56;
        return static_cast<UInt8>(packed);
    #endif
    }

    template<int W>
    inline Packet<float, W> MaskedLoadAligned(const float* p, USize count) noexcept {
        if constexpr (W == 8) {
            if (count >= 8) {
                return { _mm256_load_ps(p) };
            } else {
                alignas(32) float temp[8] = {};
                for (USize i = 0; i < count; ++i) { temp[i] = p[i]; }
                return { _mm256_load_ps(temp) };
            }
        } else if constexpr (W == 4) {
            if (count >= 4) {
                return { _mm_load_ps(p) };
            } else {
                alignas(16) float temp[4] = {};
                for (USize i = 0; i < count; ++i) { temp[i] = p[i]; }
                return { _mm_load_ps(temp) };
            }
        } else {
            return count > 0 ? Packet<float, 1>{ *p } : Packet<float, 1>{ 0.0f };
        }
    }

    template<int W>
    inline Packet<float, W> MaskedLoadUnaligned(const float* p, USize count) noexcept {
        if constexpr (W == 8) {
            if (count >= 8) {
                return { _mm256_loadu_ps(p) };
            } else {
                alignas(32) float temp[8] = {};
                for (USize i = 0; i < count; ++i) { temp[i] = p[i]; }
                return { _mm256_load_ps(temp) };
            }
        } else if constexpr (W == 4) {
            if (count >= 4) {
                return { _mm_loadu_ps(p) };
            } else {
                alignas(16) float temp[4] = {};
                for (USize i = 0; i < count; ++i) { temp[i] = p[i]; }
                return { _mm_load_ps(temp) };
            }
        } else {
            return count > 0 ? Packet<float, 1>{ *p } : Packet<float, 1>{ 0.0f };
        }
    }

    template<int W>
    inline void MaskedStoreAligned(float* p, Packet<float, W> v, USize count) noexcept {
        if constexpr (W == 8) {
            if (count >= 8) {
                _mm256_store_ps(p, v.reg);
            } else if (count > 0) {
                alignas(32) float temp[8];
                _mm256_store_ps(temp, v.reg);
                for (USize i = 0; i < count; ++i) { p[i] = temp[i]; }
            }
        } else if constexpr (W == 4) {
            if (count >= 4) {
                _mm_store_ps(p, v.reg);
            } else if (count > 0) {
                alignas(16) float temp[4];
                _mm_store_ps(temp, v.reg);
                for (USize i = 0; i < count; ++i) { p[i] = temp[i]; }
            }
        } else {
            if (count > 0) { *p = v.reg; }
        }
    }

    template<int W>
    inline void MaskedStoreUnaligned(float* p, Packet<float, W> v, USize count) noexcept {
        if constexpr (W == 8) {
            if (count >= 8) {
                _mm256_storeu_ps(p, v.reg);
            } else if (count > 0) {
                alignas(32) float temp[8];
                _mm256_store_ps(temp, v.reg);
                for (USize i = 0; i < count; ++i) { p[i] = temp[i]; }
            }
        } else if constexpr (W == 4) {
            if (count >= 4) {
                _mm_storeu_ps(p, v.reg);
            } else if (count > 0) {
                alignas(16) float temp[4];
                _mm_store_ps(temp, v.reg);
                for (USize i = 0; i < count; ++i) { p[i] = temp[i]; }
            }
        } else {
            if (count > 0) { *p = v.reg; }
        }
    }

    template<>
    struct Packet<double, 4> {
        __m256d reg;
        inline Packet operator+(Packet rhs) const noexcept { return { _mm256_add_pd(reg, rhs.reg) }; }
        inline Packet operator-(Packet rhs) const noexcept { return { _mm256_sub_pd(reg, rhs.reg) }; }
        inline Packet operator*(Packet rhs) const noexcept { return { _mm256_mul_pd(reg, rhs.reg) }; }
        static inline Packet LoadAligned(const double* p) noexcept { return { _mm256_load_pd(p) }; }
        static inline Packet LoadUnaligned(const double* p) noexcept { return { _mm256_loadu_pd(p) }; }
        static inline void StoreAligned(double* p, Packet v) noexcept { _mm256_store_pd(p, v.reg); }
        static inline void StoreUnaligned(double* p, Packet v) noexcept { _mm256_storeu_pd(p, v.reg); }
    };

    inline Packet<double, 4> FMA(Packet<double, 4> a, Packet<double, 4> b, Packet<double, 4> c) noexcept {
    #if defined(__FMA__)
        return { _mm256_fmadd_pd(a.reg, b.reg, c.reg) };
    #else
        return { _mm256_add_pd(_mm256_mul_pd(a.reg, b.reg), c.reg) };
    #endif
    }

    template<>
    struct Packet<double, 2> {
        __m128d reg;
        inline Packet operator+(Packet rhs) const noexcept { return { _mm_add_pd(reg, rhs.reg) }; }
        inline Packet operator-(Packet rhs) const noexcept { return { _mm_sub_pd(reg, rhs.reg) }; }
        inline Packet operator*(Packet rhs) const noexcept { return { _mm_mul_pd(reg, rhs.reg) }; }
        static inline Packet LoadAligned(const double* p) noexcept { return { _mm_load_pd(p) }; }
        static inline Packet LoadUnaligned(const double* p) noexcept { return { _mm_loadu_pd(p) }; }
        static inline void StoreAligned(double* p, Packet v) noexcept { _mm_store_pd(p, v.reg); }
        static inline void StoreUnaligned(double* p, Packet v) noexcept { _mm_storeu_pd(p, v.reg); }
    };

    inline Packet<double, 2> FMA(Packet<double, 2> a, Packet<double, 2> b, Packet<double, 2> c) noexcept {
    #if defined(__FMA__)
        return { _mm_fmadd_pd(a.reg, b.reg, c.reg) };
    #else
        return { _mm_add_pd(_mm_mul_pd(a.reg, b.reg), c.reg) };
    #endif
    }

    template<>
    struct Packet<double, 1> {
        double reg;
        inline Packet operator+(Packet rhs) const noexcept { return { reg + rhs.reg }; }
        inline Packet operator-(Packet rhs) const noexcept { return { reg - rhs.reg }; }
        inline Packet operator*(Packet rhs) const noexcept { return { reg * rhs.reg }; }
        static inline Packet LoadAligned(const double* p) noexcept { return { *p }; }
        static inline Packet LoadUnaligned(const double* p) noexcept { return { *p }; }
        static inline void StoreAligned(double* p, Packet v) noexcept { *p = v.reg; }
        static inline void StoreUnaligned(double* p, Packet v) noexcept { *p = v.reg; }
    };

    inline Packet<double, 1> FMA(Packet<double, 1> a, Packet<double, 1> b, Packet<double, 1> c) noexcept {
        return { a.reg * b.reg + c.reg };
    }

    template<>
    struct Packet<float, 2> {
        __m128 reg;
        inline Packet operator+(Packet rhs) const noexcept { return { _mm_add_ps(reg, rhs.reg) }; }
        inline Packet operator-(Packet rhs) const noexcept { return { _mm_sub_ps(reg, rhs.reg) }; }
        inline Packet operator*(Packet rhs) const noexcept { return { _mm_mul_ps(reg, rhs.reg) }; }
        static inline Packet LoadAligned(const float* p) noexcept { 
            alignas(16) float temp[4] = {p[0], p[1], 0.0f, 0.0f};
            return { _mm_load_ps(temp) }; 
        }
        static inline Packet LoadUnaligned(const float* p) noexcept { 
            __m128 t = _mm_setzero_ps();
            t = _mm_loadl_pi(t, reinterpret_cast<const __m64*>(p));
            return { t };
        }
        static inline void StoreAligned(float* p, Packet v) noexcept { 
            alignas(16) float temp[4];
            _mm_store_ps(temp, v.reg);
            p[0] = temp[0]; p[1] = temp[1];
        }
        static inline void StoreUnaligned(float* p, Packet v) noexcept { 
            _mm_storel_pi(reinterpret_cast<__m64*>(p), v.reg);
        }
    };

    inline Packet<float, 2> FMA(Packet<float, 2> a, Packet<float, 2> b, Packet<float, 2> c) noexcept {
    #if defined(__FMA__)
        return { _mm_fmadd_ps(a.reg, b.reg, c.reg) };
    #else
        return { _mm_add_ps(_mm_mul_ps(a.reg, b.reg), c.reg) };
    #endif
    }

    inline Packet<float, 4> ConvertDoubleToFloat(Packet<double, 4> d) noexcept {
        return { _mm256_cvtpd_ps(d.reg) };
    }

    inline Packet<float, 2> ConvertDoubleToFloat(Packet<double, 2> d) noexcept {
        return { _mm_cvtpd_ps(d.reg) };
    }

    inline Packet<float, 1> ConvertDoubleToFloat(Packet<double, 1> d) noexcept {
        return { static_cast<float>(d.reg) };
    }
}