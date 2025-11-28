module;
export module SIMD:Packet;
import Language;
import <immintrin.h>;

export namespace SIMD {
    template<typename T, int W>
    struct Packet;

    template<> struct Packet<float, 8> {
        __m256 reg;
        inline Packet operator+(Packet rhs) const noexcept { return { _mm256_add_ps(reg, rhs.reg) }; }
        inline Packet operator-(Packet rhs) const noexcept { return { _mm256_sub_ps(reg, rhs.reg) }; }
        inline Packet operator*(Packet rhs) const noexcept { return { _mm256_mul_ps(reg, rhs.reg) }; }
        inline Packet operator/(Packet rhs) const noexcept { return { _mm256_div_ps(reg, rhs.reg) }; }
        static inline Packet LoadAligned(const float* p) noexcept { return { _mm256_load_ps(p) }; }
        static inline Packet LoadUnaligned(const float* p) noexcept { return { _mm256_loadu_ps(p) }; }
        static inline void StoreAligned(float* p, Packet v) noexcept { _mm256_store_ps(p, v.reg); }
        static inline void StoreUnaligned(float* p, Packet v) noexcept { _mm256_storeu_ps(p, v.reg); }
    };

    template<> struct Packet<float, 4> {
        __m128 reg;
        inline Packet operator+(Packet rhs) const noexcept { return { _mm_add_ps(reg, rhs.reg) }; }
        inline Packet operator-(Packet rhs) const noexcept { return { _mm_sub_ps(reg, rhs.reg) }; }
        inline Packet operator*(Packet rhs) const noexcept { return { _mm_mul_ps(reg, rhs.reg) }; }
        inline Packet operator/(Packet rhs) const noexcept { return { _mm_div_ps(reg, rhs.reg) }; }
        static inline Packet LoadAligned(const float* p) noexcept { return { _mm_load_ps(p) }; }
        static inline Packet LoadUnaligned(const float* p) noexcept { return { _mm_loadu_ps(p) }; }
        static inline void StoreAligned(float* p, Packet v) noexcept { _mm_store_ps(p, v.reg); }
        static inline void StoreUnaligned(float* p, Packet v) noexcept { _mm_storeu_ps(p, v.reg); }
    };

    template<> struct Packet<float, 2> {
        __m128 reg;
        inline Packet operator+(Packet rhs) const noexcept { return { _mm_add_ps(reg, rhs.reg) }; }
        inline Packet operator-(Packet rhs) const noexcept { return { _mm_sub_ps(reg, rhs.reg) }; }
        inline Packet operator*(Packet rhs) const noexcept { return { _mm_mul_ps(reg, rhs.reg) }; }
        inline Packet operator/(Packet rhs) const noexcept { return { _mm_div_ps(reg, rhs.reg) }; }
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

    template<> struct Packet<float, 1> {
        float reg;
        inline Packet operator+(Packet rhs) const noexcept { return { reg + rhs.reg }; }
        inline Packet operator-(Packet rhs) const noexcept { return { reg - rhs.reg }; }
        inline Packet operator*(Packet rhs) const noexcept { return { reg * rhs.reg }; }
        inline Packet operator/(Packet rhs) const noexcept { return { reg / rhs.reg }; }
        static inline Packet LoadAligned(const float* p) noexcept { return { *p }; }
        static inline Packet LoadUnaligned(const float* p) noexcept { return { *p }; }
        static inline void StoreAligned(float* p, Packet v) noexcept { *p = v.reg; }
        static inline void StoreUnaligned(float* p, Packet v) noexcept { *p = v.reg; }
    };

    template<> struct Packet<double, 4> {
        __m256d reg;
        inline Packet operator+(Packet rhs) const noexcept { return { _mm256_add_pd(reg, rhs.reg) }; }
        inline Packet operator-(Packet rhs) const noexcept { return { _mm256_sub_pd(reg, rhs.reg) }; }
        inline Packet operator*(Packet rhs) const noexcept { return { _mm256_mul_pd(reg, rhs.reg) }; }
        inline Packet operator/(Packet rhs) const noexcept { return { _mm256_div_pd(reg, rhs.reg) }; }
        static inline Packet LoadAligned(const double* p) noexcept { return { _mm256_load_pd(p) }; }
        static inline Packet LoadUnaligned(const double* p) noexcept { return { _mm256_loadu_pd(p) }; }
        static inline void StoreAligned(double* p, Packet v) noexcept { _mm256_store_pd(p, v.reg); }
        static inline void StoreUnaligned(double* p, Packet v) noexcept { _mm256_storeu_pd(p, v.reg); }
    };

    template<> struct Packet<double, 2> {
        __m128d reg;
        inline Packet operator+(Packet rhs) const noexcept { return { _mm_add_pd(reg, rhs.reg) }; }
        inline Packet operator-(Packet rhs) const noexcept { return { _mm_sub_pd(reg, rhs.reg) }; }
        inline Packet operator*(Packet rhs) const noexcept { return { _mm_mul_pd(reg, rhs.reg) }; }
        inline Packet operator/(Packet rhs) const noexcept { return { _mm_div_pd(reg, rhs.reg) }; }
        static inline Packet LoadAligned(const double* p) noexcept { return { _mm_load_pd(p) }; }
        static inline Packet LoadUnaligned(const double* p) noexcept { return { _mm_loadu_pd(p) }; }
        static inline void StoreAligned(double* p, Packet v) noexcept { _mm_store_pd(p, v.reg); }
        static inline void StoreUnaligned(double* p, Packet v) noexcept { _mm_storeu_pd(p, v.reg); }
    };

    template<> struct Packet<double, 1> {
        double reg;
        inline Packet operator+(Packet rhs) const noexcept { return { reg + rhs.reg }; }
        inline Packet operator-(Packet rhs) const noexcept { return { reg - rhs.reg }; }
        inline Packet operator*(Packet rhs) const noexcept { return { reg * rhs.reg }; }
        inline Packet operator/(Packet rhs) const noexcept { return { reg / rhs.reg }; }
        static inline Packet LoadAligned(const double* p) noexcept { return { *p }; }
        static inline Packet LoadUnaligned(const double* p) noexcept { return { *p }; }
        static inline void StoreAligned(double* p, Packet v) noexcept { *p = v.reg; }
        static inline void StoreUnaligned(double* p, Packet v) noexcept { *p = v.reg; }
    };
}

