module;
export module Cap.Math:Transcendental;
import Language;
import SIMD;
import :Storage;
import :Compute;
import <cmath>;
// 下沉 SIMD 具体实现至 Primitive.SIMD，避免在能力层直接使用平台内建
export namespace Cap {
    struct Transcendental {
        template<int W>
        static inline SIMD::Packet<float, W> Sin(SIMD::Packet<float, W> x) noexcept {
            auto pi      = SIMD::Set1<W>(3.14159265358979323846f);
            auto half_pi = SIMD::Set1<W>(1.57079632679489661923f);
            auto y = x;
            auto gt = SIMD::GreaterThan(y, half_pi);
            y = SIMD::Select(gt, y - pi, y);
            auto lt = SIMD::Less(y, SIMD::Set1<W>(-1.57079632679489661923f));
            y = SIMD::Select(lt, y + pi, y);
            auto y2 = y * y;
            auto c3 = SIMD::Set1<W>(-0.16666667163372039795f);
            auto c5 = SIMD::Set1<W>(0.00833333109718561172f);
            auto c7 = SIMD::Set1<W>(-0.00019840873932297444f);
            auto p  = SIMD::FMA(c7, y2, c5);
            p       = SIMD::FMA(p, y2, c3);
            return SIMD::FMA(p, y2 * y, y);
        }
        template<int W>
        static inline SIMD::Packet<float, W> Cos(SIMD::Packet<float, W> x) noexcept {
            auto pi      = SIMD::Set1<W>(3.14159265358979323846f);
            auto half_pi = SIMD::Set1<W>(1.57079632679489661923f);
            auto y = x;
            auto gt = SIMD::GreaterThan(y, half_pi);
            y = SIMD::Select(gt, y - pi, y);
            auto lt = SIMD::Less(y, SIMD::Set1<W>(-1.57079632679489661923f));
            y = SIMD::Select(lt, y + pi, y);
            auto y2 = y * y;
            auto c2 = SIMD::Set1<W>(-0.5f);
            auto c4 = SIMD::Set1<W>(0.04166666790843009949f);
            auto c6 = SIMD::Set1<W>(-0.0013888889225190282f);
            auto p  = SIMD::FMA(c6, y2, c4);
            p       = SIMD::FMA(p, y2, c2);
            return SIMD::FMA(p, y2, SIMD::Set1<W>(1.0f));
        }
        template<int W>
        static inline SIMD::Packet<float, W> Tan(SIMD::Packet<float, W> x) noexcept {
            auto s = Sin<W>(x);
            auto c = Cos<W>(x);
            auto zero = SIMD::Set1<W>(0.0f);
            auto safe = SIMD::GreaterThan(SIMD::Abs(c), SIMD::Set1<W>(1e-6f));
            auto r = s * SIMD::Recp(c);
            return SIMD::Select(safe, r, zero);
        }
        template<int W>
        static inline SIMD::Packet<float, W> Sqrt(SIMD::Packet<float, W> x) noexcept {
            auto zero = SIMD::Set1<W>(0.0f);
            auto inv  = SIMD::Rsqrt(x);
            auto half = SIMD::Set1<W>(0.5f);
            auto thr  = SIMD::Set1<W>(1.5f);
            auto yy   = inv * inv;
            auto xyy  = x * yy;
            auto inn  = thr - half * xyy;
            inv = inv * inn;
            auto r = x * inv;
            auto m = SIMD::GreaterThan(x, zero);
            return SIMD::Select(m, r, zero);
        }
        template<int W>
        static inline SIMD::Packet<float, W> Exp(SIMD::Packet<float, W> x) noexcept { return SIMD::Exp(x); }
        template<int W>
        static inline SIMD::Packet<float, W> Log(SIMD::Packet<float, W> x) noexcept { return SIMD::Log(x); }
        template<int W>
        static inline SIMD::Packet<float, W> Log2(SIMD::Packet<float, W> x) noexcept {
            constexpr float kInvLn2 = 1.4426950408889634f;
            return SIMD::Log(x) * SIMD::Set1<W>(kInvLn2);
        }
        template<int W>
        static inline SIMD::Packet<float, W> Pow(SIMD::Packet<float, W> x, SIMD::Packet<float, W> y) noexcept {
            // 仅在 x>0 时定义，其他情况简单回退
            auto zero = SIMD::Set1<W>(0.0f);
            auto mask = SIMD::GreaterThan(x, zero);
            auto res  = SIMD::Exp(y * SIMD::Log(x));
            return SIMD::Select(mask, res, zero);
        }
        template<int W>
        static inline SIMD::Packet<float, W> Atan2(SIMD::Packet<float, W> y, SIMD::Packet<float, W> x) noexcept {
            if constexpr (W == 8) {
                float yy[8], xx[8], rr[8];
                SIMD::Packet<float,8>::StoreUnaligned(yy, y);
                SIMD::Packet<float,8>::StoreUnaligned(xx, x);
                for (int i=0;i<8;++i) rr[i] = std::atan2(yy[i], xx[i]);
                return SIMD::Packet<float,8>::LoadUnaligned(rr);
            } else if constexpr (W == 4) {
                float yy[4], xx[4], rr[4];
                SIMD::Packet<float,4>::StoreUnaligned(yy, y);
                SIMD::Packet<float,4>::StoreUnaligned(xx, x);
                for (int i=0;i<4;++i) rr[i] = std::atan2(yy[i], xx[i]);
                return SIMD::Packet<float,4>::LoadUnaligned(rr);
            } else if constexpr (W == 2) {
                float yy[2], xx[2], rr[2];
                SIMD::Packet<float,2>::StoreUnaligned(yy, y);
                SIMD::Packet<float,2>::StoreUnaligned(xx, x);
                for (int i=0;i<2;++i) rr[i] = std::atan2(yy[i], xx[i]);
                return SIMD::Packet<float,2>::LoadUnaligned(rr);
            } else {
                float yy, xx; SIMD::Packet<float,1>::StoreAligned(&yy, y); SIMD::Packet<float,1>::StoreAligned(&xx, x);
                float r = std::atan2(yy, xx);
                return SIMD::Packet<float,1>::LoadAligned(&r);
            }
        }
        template<int W>
        static inline Vector3Packet<W> Sin(Vector3Packet<W> v) noexcept {
            return { Sin<W>(v.xs), Sin<W>(v.ys), Sin<W>(v.zs) };
        }
        template<int W>
        static inline Vector3Packet<W> Cos(Vector3Packet<W> v) noexcept {
            return { Cos<W>(v.xs), Cos<W>(v.ys), Cos<W>(v.zs) };
        }
        template<int W>
        static inline Vector3Packet<W> Tan(Vector3Packet<W> v) noexcept {
            return { Tan<W>(v.xs), Tan<W>(v.ys), Tan<W>(v.zs) };
        }
        template<int W>
        static inline Vector3Packet<W> Sqrt(Vector3Packet<W> v) noexcept {
            return { Sqrt<W>(v.xs), Sqrt<W>(v.ys), Sqrt<W>(v.zs) };
        }
        template<int W>
        static inline Vector3Packet<W> Exp(Vector3Packet<W> v) noexcept {
            return { Exp<W>(v.xs), Exp<W>(v.ys), Exp<W>(v.zs) };
        }
        template<int W>
        static inline Vector3Packet<W> Log(Vector3Packet<W> v) noexcept {
            return { Log<W>(v.xs), Log<W>(v.ys), Log<W>(v.zs) };
        }
        template<int W>
        static inline Vector3Packet<W> Log2(Vector3Packet<W> v) noexcept {
            return { Log2<W>(v.xs), Log2<W>(v.ys), Log2<W>(v.zs) };
        }
        template<int W>
        static inline Vector3Packet<W> Pow(Vector3Packet<W> x, Vector3Packet<W> y) noexcept {
            return { Pow<W>(x.xs, y.xs), Pow<W>(x.ys, y.ys), Pow<W>(x.zs, y.zs) };
        }
        template<int W>
        static inline Vector3Packet<W> Atan2(Vector3Packet<W> y, Vector3Packet<W> x) noexcept {
            return { Atan2<W>(y.xs, x.xs), Atan2<W>(y.ys, x.ys), Atan2<W>(y.zs, x.zs) };
        }
    };
}
