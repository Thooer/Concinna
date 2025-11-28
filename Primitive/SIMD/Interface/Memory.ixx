module;
export module SIMD:Memory;
import Language;
import <immintrin.h>;
import :Packet;

export namespace SIMD {
    template<int W>
    inline Packet<float, W> MaskedLoadAligned(const float* p, USize count) noexcept {
        if constexpr (W == 8) {
            if (count >= 8) { return { _mm256_load_ps(p) }; }
            alignas(32) float temp[8] = {}; for (USize i = 0; i < count; ++i) temp[i] = p[i]; return { _mm256_load_ps(temp) };
        } else if constexpr (W == 4) {
            if (count >= 4) { return { _mm_load_ps(p) }; }
            alignas(16) float temp[4] = {}; for (USize i = 0; i < count; ++i) temp[i] = p[i]; return { _mm_load_ps(temp) };
        } else if constexpr (W == 2) {
            alignas(16) float temp[4] = {}; for (USize i = 0; i < count && i < 2; ++i) temp[i] = p[i]; return { _mm_load_ps(temp) };
        } else {
            return count > 0 ? Packet<float, 1>{ *p } : Packet<float, 1>{ 0.0f };
        }
    }

    template<int W>
    inline Packet<float, W> MaskedLoadUnaligned(const float* p, USize count) noexcept {
        if constexpr (W == 8) {
            if (count >= 8) { return { _mm256_loadu_ps(p) }; }
            alignas(32) float temp[8] = {}; for (USize i = 0; i < count; ++i) temp[i] = p[i]; return { _mm256_load_ps(temp) };
        } else if constexpr (W == 4) {
            if (count >= 4) { return { _mm_loadu_ps(p) }; }
            alignas(16) float temp[4] = {}; for (USize i = 0; i < count; ++i) temp[i] = p[i]; return { _mm_load_ps(temp) };
        } else if constexpr (W == 2) {
            alignas(16) float temp[4] = {}; for (USize i = 0; i < count && i < 2; ++i) temp[i] = p[i]; return { _mm_load_ps(temp) };
        } else {
            return count > 0 ? Packet<float, 1>{ *p } : Packet<float, 1>{ 0.0f };
        }
    }

    template<int W>
    inline void MaskedStoreAligned(float* p, Packet<float, W> v, USize count) noexcept {
        if constexpr (W == 8) {
            if (count >= 8) { _mm256_store_ps(p, v.reg); }
            else if (count > 0) { alignas(32) float temp[8]; _mm256_store_ps(temp, v.reg); for (USize i = 0; i < count; ++i) p[i] = temp[i]; }
        } else if constexpr (W == 4) {
            if (count >= 4) { _mm_store_ps(p, v.reg); }
            else if (count > 0) { alignas(16) float temp[4]; _mm_store_ps(temp, v.reg); for (USize i = 0; i < count; ++i) p[i] = temp[i]; }
        } else if constexpr (W == 2) {
            if (count > 0) { alignas(16) float temp[4]; _mm_store_ps(temp, v.reg); for (USize i = 0; i < count && i < 2; ++i) p[i] = temp[i]; }
        } else { if (count > 0) { *p = v.reg; } }
    }

    template<int W>
    inline void MaskedStoreUnaligned(float* p, Packet<float, W> v, USize count) noexcept {
        if constexpr (W == 8) {
            if (count >= 8) { _mm256_storeu_ps(p, v.reg); }
            else if (count > 0) { alignas(32) float temp[8]; _mm256_store_ps(temp, v.reg); for (USize i = 0; i < count; ++i) p[i] = temp[i]; }
        } else if constexpr (W == 4) {
            if (count >= 4) { _mm_storeu_ps(p, v.reg); }
            else if (count > 0) { alignas(16) float temp[4]; _mm_store_ps(temp, v.reg); for (USize i = 0; i < count; ++i) p[i] = temp[i]; }
        } else if constexpr (W == 2) {
            if (count > 0) { alignas(16) float temp[4]; _mm_store_ps(temp, v.reg); for (USize i = 0; i < count && i < 2; ++i) p[i] = temp[i]; }
        } else { if (count > 0) { *p = v.reg; } }
    }

    template<int W>
    inline Packet<float, W> Gather(const float* base, const int* idx) noexcept {
        if constexpr (W == 8) {
        #if defined(__AVX2__)
            __m256i vi = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(idx));
            return { _mm256_i32gather_ps(base, vi, 4) };
        #else
            alignas(32) float temp[8]; for (int k=0;k<8;++k) temp[k] = base[idx[k]]; return { _mm256_load_ps(temp) };
        #endif
        } else if constexpr (W == 4) {
        #if defined(__AVX2__)
            __m128i vi = _mm_loadu_si128(reinterpret_cast<const __m128i*>(idx));
            return { _mm_i32gather_ps(base, vi, 4) };
        #else
            alignas(16) float temp[4]; for (int k=0;k<4;++k) temp[k] = base[idx[k]]; return { _mm_load_ps(temp) };
        #endif
        } else if constexpr (W == 2) {
            alignas(16) float temp[4] = { base[idx[0]], base[idx[1]], 0.0f, 0.0f };
            return { _mm_load_ps(temp) };
        } else {
            return { base[idx[0]] };
        }
    }

    template<int W>
    inline void Scatter(float* base, Packet<float, W> v, const int* idx) noexcept {
        if constexpr (W == 8) {
            alignas(32) float temp[8]; _mm256_store_ps(temp, v.reg); for (int k=0;k<8;++k) base[idx[k]] = temp[k];
        } else if constexpr (W == 4) {
            alignas(16) float temp[4]; _mm_store_ps(temp, v.reg); for (int k=0;k<4;++k) base[idx[k]] = temp[k];
        } else if constexpr (W == 2) {
            alignas(16) float temp[4]; _mm_store_ps(temp, v.reg); base[idx[0]] = temp[0]; base[idx[1]] = temp[1];
        } else {
            base[idx[0]] = v.reg;
        }
    }
}

