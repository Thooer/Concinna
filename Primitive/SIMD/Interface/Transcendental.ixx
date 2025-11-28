module;
export module SIMD:Transcendental;
import Language;
import <immintrin.h>;
import <cmath>;
import :Packet;

export namespace SIMD {
    template<int W>
    inline Packet<float, W> Exp(Packet<float, W> x) noexcept {
        if constexpr (W == 8) {
            const __m256 vx = x.reg;
            const __m256 invln2 = _mm256_set1_ps(1.4426950408889634f);
            const __m256 ln2    = _mm256_set1_ps(0.6931471805599453f);
            __m256 fk = _mm256_mul_ps(vx, invln2);
            __m256i ik = _mm256_cvtps_epi32(fk);
            __m256 kf = _mm256_cvtepi32_ps(ik);
            __m256 t  = _mm256_sub_ps(vx, _mm256_mul_ps(kf, ln2));
            __m256 c5 = _mm256_set1_ps(1.0f/120.0f);
            __m256 c4 = _mm256_set1_ps(1.0f/24.0f);
            __m256 c3 = _mm256_set1_ps(1.0f/6.0f);
            __m256 c2 = _mm256_set1_ps(0.5f);
            __m256 c1 = _mm256_set1_ps(1.0f);
            __m256 p  = c5;
            p = _mm256_fmadd_ps(p, t, c4);
            p = _mm256_fmadd_ps(p, t, c3);
            p = _mm256_fmadd_ps(p, t, c2);
            p = _mm256_fmadd_ps(p, t, c1);
            __m256 poly = _mm256_fmadd_ps(p, t, _mm256_set1_ps(1.0f));
            __m256i e   = _mm256_add_epi32(ik, _mm256_set1_epi32(127));
            e = _mm256_slli_epi32(e, 23);
            __m256 exp2k = _mm256_castsi256_ps(e);
            __m256 res = _mm256_mul_ps(exp2k, poly);
            return { res };
        } else if constexpr (W == 4) {
            const __m128 vx = x.reg;
            const __m128 invln2 = _mm_set1_ps(1.4426950408889634f);
            const __m128 ln2    = _mm_set1_ps(0.6931471805599453f);
            __m128 fk = _mm_mul_ps(vx, invln2);
            __m128i ik = _mm_cvtps_epi32(fk);
            __m128 kf = _mm_cvtepi32_ps(ik);
            __m128 t  = _mm_sub_ps(vx, _mm_mul_ps(kf, ln2));
            __m128 c5 = _mm_set1_ps(1.0f/120.0f);
            __m128 c4 = _mm_set1_ps(1.0f/24.0f);
            __m128 c3 = _mm_set1_ps(1.0f/6.0f);
            __m128 c2 = _mm_set1_ps(0.5f);
            __m128 c1 = _mm_set1_ps(1.0f);
            __m128 p  = c5;
            p = _mm_fmadd_ps(p, t, c4);
            p = _mm_fmadd_ps(p, t, c3);
            p = _mm_fmadd_ps(p, t, c2);
            p = _mm_fmadd_ps(p, t, c1);
            __m128 poly = _mm_fmadd_ps(p, t, _mm_set1_ps(1.0f));
            __m128i e   = _mm_add_epi32(ik, _mm_set1_epi32(127));
            e = _mm_slli_epi32(e, 23);
            __m128 exp2k = _mm_castsi128_ps(e);
            __m128 res = _mm_mul_ps(exp2k, poly);
            return { res };
        } else if constexpr (W == 2) {
            float in[2]; float out[2];
            Packet<float,2>::StoreUnaligned(in, x);
            for (int i=0;i<2;++i) { out[i] = std::exp(in[i]); }
            return Packet<float,2>::LoadUnaligned(out);
        } else {
            float v; Packet<float,1>::StoreAligned(&v, x);
            v = std::exp(v);
            return Packet<float,1>::LoadAligned(&v);
        }
    }

    template<int W>
    inline Packet<float, W> Log(Packet<float, W> x) noexcept {
        if constexpr (W == 8) {
            __m256 vx = x.reg;
            __m256 minv = _mm256_set1_ps(1e-30f);
            vx = _mm256_max_ps(vx, minv);
            __m256i ix = _mm256_castps_si256(vx);
            __m256i e  = _mm256_srli_epi32(ix, 23);
            e = _mm256_and_si256(e, _mm256_set1_epi32(0xFF));
            e = _mm256_sub_epi32(e, _mm256_set1_epi32(127));
            __m256i mant = _mm256_and_si256(ix, _mm256_set1_epi32(0x7FFFFF));
            mant = _mm256_or_si256(mant, _mm256_set1_epi32(127 << 23));
            __m256 m = _mm256_castsi256_ps(mant);
            __m256 t = _mm256_sub_ps(m, _mm256_set1_ps(1.0f));
            __m256 c1 = _mm256_set1_ps(1.0f);
            __m256 c2 = _mm256_set1_ps(-0.5f);
            __m256 c3 = _mm256_set1_ps(1.0f/3.0f);
            __m256 c4 = _mm256_set1_ps(-1.0f/4.0f);
            __m256 c5 = _mm256_set1_ps(1.0f/5.0f);
            __m256 p  = c5;
            p = _mm256_fmadd_ps(p, t, c4);
            p = _mm256_fmadd_ps(p, t, c3);
            p = _mm256_fmadd_ps(p, t, c2);
            p = _mm256_fmadd_ps(p, t, c1);
            __m256 poly = _mm256_mul_ps(p, t);
            __m256 ln2 = _mm256_set1_ps(0.6931471805599453f);
            __m256 ef = _mm256_cvtepi32_ps(e);
            __m256 res = _mm256_fmadd_ps(ef, ln2, poly);
            return { res };
        } else if constexpr (W == 4) {
            __m128 vx = x.reg;
            __m128 minv = _mm_set1_ps(1e-30f);
            vx = _mm_max_ps(vx, minv);
            __m128i ix = _mm_castps_si128(vx);
            __m128i e  = _mm_srli_epi32(ix, 23);
            e = _mm_and_si128(e, _mm_set1_epi32(0xFF));
            e = _mm_sub_epi32(e, _mm_set1_epi32(127));
            __m128i mant = _mm_and_si128(ix, _mm_set1_epi32(0x7FFFFF));
            mant = _mm_or_si128(mant, _mm_set1_epi32(127 << 23));
            __m128 m = _mm_castsi128_ps(mant);
            __m128 t = _mm_sub_ps(m, _mm_set1_ps(1.0f));
            __m128 c1 = _mm_set1_ps(1.0f);
            __m128 c2 = _mm_set1_ps(-0.5f);
            __m128 c3 = _mm_set1_ps(1.0f/3.0f);
            __m128 c4 = _mm_set1_ps(-1.0f/4.0f);
            __m128 c5 = _mm_set1_ps(1.0f/5.0f);
            __m128 p  = c5;
            p = _mm_fmadd_ps(p, t, c4);
            p = _mm_fmadd_ps(p, t, c3);
            p = _mm_fmadd_ps(p, t, c2);
            p = _mm_fmadd_ps(p, t, c1);
            __m128 poly = _mm_mul_ps(p, t);
            __m128 ln2 = _mm_set1_ps(0.6931471805599453f);
            __m128 ef = _mm_cvtepi32_ps(e);
            __m128 res = _mm_fmadd_ps(ef, ln2, poly);
            return { res };
        } else if constexpr (W == 2) {
            float in[2]; float out[2];
            Packet<float,2>::StoreUnaligned(in, x);
            for (int i=0;i<2;++i) { out[i] = in[i] > 0.0f ? std::log(in[i]) : -INFINITY; }
            return Packet<float,2>::LoadUnaligned(out);
        } else {
            float v; Packet<float,1>::StoreAligned(&v, x);
            v = v > 0.0f ? std::log(v) : -INFINITY;
            return Packet<float,1>::LoadAligned(&v);
        }
    }
}
