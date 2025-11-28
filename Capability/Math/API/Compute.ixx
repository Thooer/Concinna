module;
export module Cap.Math:Compute;
import Language;
import SIMD;
import :Storage;
export namespace Cap {
    template<int W>
    struct Vector3Packet {
        SIMD::Packet<float, W> xs;
        SIMD::Packet<float, W> ys;
        SIMD::Packet<float, W> zs;
        static inline Vector3Packet Add(Vector3Packet a, Vector3Packet b) noexcept {
            return { a.xs + b.xs, a.ys + b.ys, a.zs + b.zs };
        }
        static inline Vector3Packet Sub(Vector3Packet a, Vector3Packet b) noexcept {
            return { a.xs - b.xs, a.ys - b.ys, a.zs - b.zs };
        }
        static inline Vector3Packet Mul(Vector3Packet a, SIMD::Packet<float, W> s) noexcept {
            return { a.xs * s, a.ys * s, a.zs * s };
        }
        static inline SIMD::Packet<float, W> Dot(Vector3Packet a, Vector3Packet b) noexcept {
            auto r = a.xs * b.xs;
            r = SIMD::FMA(a.ys, b.ys, r);
            r = SIMD::FMA(a.zs, b.zs, r);
            return r;
        }
        static inline SIMD::Packet<float, W> LengthSquared(Vector3Packet a) noexcept {
            return Dot(a, a);
        }
        static inline SIMD::Packet<float, W> Length(Vector3Packet a) noexcept {
            auto len2 = Dot(a, a);
            auto inv  = SIMD::Rsqrt(len2);
            auto half = SIMD::Set1<W>(0.5f);
            auto thr  = SIMD::Set1<W>(1.5f);
            auto yy   = inv * inv;
            auto xyy  = len2 * yy;
            auto inn  = thr - half * xyy;
            inv = inv * inn;
            return len2 * inv;
        }
        static inline SIMD::Packet<float, W> Distance(Vector3Packet a, Vector3Packet b) noexcept {
            return Length(Sub(b, a));
        }
        static inline Vector3Packet Cross(Vector3Packet a, Vector3Packet b) noexcept {
            return { a.ys * b.zs - a.zs * b.ys,
                     a.zs * b.xs - a.xs * b.zs,
                     a.xs * b.ys - a.ys * b.xs };
        }
        static inline Vector3Packet Normalize(Vector3Packet a) noexcept {
            auto len2 = Dot(a, a);
            auto inv  = SIMD::Rsqrt(len2);
            auto half = SIMD::Set1<W>(0.5f);
            auto thr  = SIMD::Set1<W>(1.5f);
            auto yy   = inv * inv;
            auto xyy  = len2 * yy;
            auto inn  = thr - half * xyy;
            inv = inv * inn;
            auto zero = a.xs - a.xs;
            auto m    = SIMD::GreaterThan(len2, zero);
            return { SIMD::Select(m, a.xs * inv, zero),
                     SIMD::Select(m, a.ys * inv, zero),
                     SIMD::Select(m, a.zs * inv, zero) };
        }
        static inline Vector3Packet Lerp(Vector3Packet a, Vector3Packet b, SIMD::Packet<float, W> t) noexcept {
            auto one_minus_t = SIMD::Set1<W>(1.0f) - t;
            return { SIMD::FMA(b.xs, t, a.xs * one_minus_t),
                     SIMD::FMA(b.ys, t, a.ys * one_minus_t),
                     SIMD::FMA(b.zs, t, a.zs * one_minus_t) };
        }
        static inline Vector3Packet Min(Vector3Packet a, Vector3Packet b) noexcept {
            return { SIMD::Min(a.xs, b.xs), SIMD::Min(a.ys, b.ys), SIMD::Min(a.zs, b.zs) };
        }
        static inline Vector3Packet Max(Vector3Packet a, Vector3Packet b) noexcept {
            return { SIMD::Max(a.xs, b.xs), SIMD::Max(a.ys, b.ys), SIMD::Max(a.zs, b.zs) };
        }
        static inline Vector3Packet Clamp(Vector3Packet v, Vector3Packet lo, Vector3Packet hi) noexcept {
            return Max(lo, Min(v, hi));
        }
        static inline Vector3Packet Abs(Vector3Packet a) noexcept {
            return { SIMD::Abs(a.xs), SIMD::Abs(a.ys), SIMD::Abs(a.zs) };
        }
        static inline Vector3Packet Project(Vector3Packet v, Vector3Packet n) noexcept {
            auto dot = Dot(v, n);
            return Mul(n, dot);
        }
        static inline Vector3Packet Reject(Vector3Packet v, Vector3Packet n) noexcept {
            return Sub(v, Project(v, n));
        }
        static inline Vector3Packet ClampLength(Vector3Packet v, SIMD::Packet<float, W> maxLen) noexcept {
            auto len = Length(v);
            auto scale = SIMD::Min(len, maxLen);
            auto zero = v.xs - v.xs;
            auto safeInv = SIMD::Rsqrt(LengthSquared(v));
            auto half = SIMD::Set1<W>(0.5f);
            auto thr = SIMD::Set1<W>(1.5f);
            auto yy = safeInv * safeInv;
            auto xyy = LengthSquared(v) * yy;
            auto inn = thr - half * xyy;
            safeInv = safeInv * inn;
            auto mask = SIMD::GreaterThan(LengthSquared(v), zero);
            return { SIMD::Select(mask, v.xs * safeInv * scale, zero),
                     SIMD::Select(mask, v.ys * safeInv * scale, zero),
                     SIMD::Select(mask, v.zs * safeInv * scale, zero) };
        }
        static inline Vector3Packet Reflect(Vector3Packet v, Vector3Packet n) noexcept {
            auto dot = Dot(v, n);
            auto two = SIMD::Set1<W>(2.0f);
            return Sub(v, Mul(n, two * dot));
        }
    };
    template<int W, USize BlockSize>
    [[nodiscard]] inline Vector3Packet<W> LoadPacketUnaligned(const SoAChunkVector3<BlockSize>& soa, USize i) noexcept {
        return { SIMD::Packet<float, W>::LoadUnaligned(&soa.xs[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.ys[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.zs[i]) };
    }
    template<int W, USize BlockSize>
    inline void StorePacketUnaligned(SoAChunkVector3<BlockSize>& soa, USize i, Vector3Packet<W> p) noexcept {
        SIMD::Packet<float, W>::StoreUnaligned(&soa.xs[i], p.xs);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.ys[i], p.ys);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.zs[i], p.zs);
    }
    template<int W, USize BlockSize>
    [[nodiscard]] inline Vector3Packet<W> LoadPacketAligned(const SoAChunkVector3<BlockSize>& soa, USize i) noexcept {
        return { SIMD::Packet<float, W>::LoadAligned(&soa.xs[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.ys[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.zs[i]) };
    }
    template<int W, USize BlockSize>
    inline void StorePacketAligned(SoAChunkVector3<BlockSize>& soa, USize i, Vector3Packet<W> p) noexcept {
        SIMD::Packet<float, W>::StoreAligned(&soa.xs[i], p.xs);
        SIMD::Packet<float, W>::StoreAligned(&soa.ys[i], p.ys);
        SIMD::Packet<float, W>::StoreAligned(&soa.zs[i], p.zs);
    }

    template<int W, USize BlockSize>
    [[nodiscard]] inline Vector3Packet<W> LoadPacketMaskedUnaligned(const SoAChunkVector3<BlockSize>& soa, USize i, USize count) noexcept {
        return { SIMD::MaskedLoadUnaligned<W>(&soa.xs[i], count),
                 SIMD::MaskedLoadUnaligned<W>(&soa.ys[i], count),
                 SIMD::MaskedLoadUnaligned<W>(&soa.zs[i], count) };
    }

    template<int W, USize BlockSize>
    inline void StorePacketMaskedUnaligned(SoAChunkVector3<BlockSize>& soa, USize i, Vector3Packet<W> p, USize count) noexcept {
        SIMD::MaskedStoreUnaligned<W>(&soa.xs[i], p.xs, count);
        SIMD::MaskedStoreUnaligned<W>(&soa.ys[i], p.ys, count);
        SIMD::MaskedStoreUnaligned<W>(&soa.zs[i], p.zs, count);
    }

    template<int W, USize BlockSize>
    [[nodiscard]] inline Vector3Packet<W> LoadPacketMaskedAligned(const SoAChunkVector3<BlockSize>& soa, USize i, USize count) noexcept {
        return { SIMD::MaskedLoadAligned<W>(&soa.xs[i], count),
                 SIMD::MaskedLoadAligned<W>(&soa.ys[i], count),
                 SIMD::MaskedLoadAligned<W>(&soa.zs[i], count) };
    }

    template<int W, USize BlockSize>
    inline void StorePacketMaskedAligned(SoAChunkVector3<BlockSize>& soa, USize i, Vector3Packet<W> p, USize count) noexcept {
        SIMD::MaskedStoreAligned<W>(&soa.xs[i], p.xs, count);
        SIMD::MaskedStoreAligned<W>(&soa.ys[i], p.ys, count);
        SIMD::MaskedStoreAligned<W>(&soa.zs[i], p.zs, count);
    }

    template<int W>
    struct QuaternionPacket {
        SIMD::Packet<float, W> xs;
        SIMD::Packet<float, W> ys;
        SIMD::Packet<float, W> zs;
        SIMD::Packet<float, W> ws;
        static inline Vector3Packet<W> RotateVector(QuaternionPacket q, Vector3Packet<W> v) noexcept {
            auto qv = Vector3Packet<W>{ q.xs, q.ys, q.zs };
            auto t  = Vector3Packet<W>::Cross(qv, v);
            auto u  = Vector3Packet<W>{ t.xs + q.ws * v.xs, t.ys + q.ws * v.ys, t.zs + q.ws * v.zs };
            auto r  = Vector3Packet<W>::Cross(qv, u);
            return { v.xs + r.xs + r.xs, v.ys + r.ys + r.ys, v.zs + r.zs + r.zs };
        }
        static inline QuaternionPacket Conjugate(QuaternionPacket q) noexcept {
            auto neg_one = SIMD::Set1<W>(-1.0f);
            return { q.xs * neg_one, q.ys * neg_one, q.zs * neg_one, q.ws };
        }
        static inline QuaternionPacket Multiply(QuaternionPacket a, QuaternionPacket b) noexcept {
            return {
                SIMD::FMA(a.ws, b.xs, SIMD::FMA(a.xs, b.ws, SIMD::FMA(a.ys, b.zs, a.zs * b.ys * SIMD::Set1<W>(-1.0f)))),
                SIMD::FMA(a.ws, b.ys, SIMD::FMA(a.ys, b.ws, SIMD::FMA(a.zs, b.xs, a.xs * b.zs * SIMD::Set1<W>(-1.0f)))),
                SIMD::FMA(a.ws, b.zs, SIMD::FMA(a.zs, b.ws, SIMD::FMA(a.xs, b.ys, a.ys * b.xs * SIMD::Set1<W>(-1.0f)))),
                SIMD::FMA(a.ws, b.ws, (a.xs * b.xs + a.ys * b.ys + a.zs * b.zs) * SIMD::Set1<W>(-1.0f))
            };
        }
        static inline SIMD::Packet<float, W> Dot(QuaternionPacket a, QuaternionPacket b) noexcept {
            auto r = a.xs * b.xs;
            r = SIMD::FMA(a.ys, b.ys, r);
            r = SIMD::FMA(a.zs, b.zs, r);
            r = SIMD::FMA(a.ws, b.ws, r);
            return r;
        }
        static inline QuaternionPacket Normalize(QuaternionPacket q) noexcept {
            auto len2 = Dot(q, q);
            auto inv  = SIMD::Rsqrt(len2);
            auto half = SIMD::Set1<W>(0.5f);
            auto thr  = SIMD::Set1<W>(1.5f);
            auto yy   = inv * inv;
            auto xyy  = len2 * yy;
            auto inn  = thr - half * xyy;
            inv = inv * inn;
            auto zero = q.xs - q.xs;
            auto m    = SIMD::GreaterThan(len2, zero);
            return { SIMD::Select(m, q.xs * inv, zero),
                     SIMD::Select(m, q.ys * inv, zero),
                     SIMD::Select(m, q.zs * inv, zero),
                     SIMD::Select(m, q.ws * inv, SIMD::Set1<W>(1.0f)) };
        }
        static inline QuaternionPacket Slerp(QuaternionPacket a, QuaternionPacket b, SIMD::Packet<float, W> t) noexcept {
            auto dot = Dot(a, b);
            auto zero = SIMD::Set1<W>(0.0f);
            auto flipMask = SIMD::GreaterThan(zero, dot);
            auto bFlipped = QuaternionPacket{
                SIMD::Select(flipMask, b.xs * SIMD::Set1<W>(-1.0f), b.xs),
                SIMD::Select(flipMask, b.ys * SIMD::Set1<W>(-1.0f), b.ys),
                SIMD::Select(flipMask, b.zs * SIMD::Set1<W>(-1.0f), b.zs),
                SIMD::Select(flipMask, b.ws * SIMD::Set1<W>(-1.0f), b.ws)
            };
            dot = SIMD::Select(flipMask, dot * SIMD::Set1<W>(-1.0f), dot);
            auto threshold = SIMD::Set1<W>(0.9995f);
            auto linearMask = SIMD::GreaterThan(dot, threshold);
            auto one_minus_t = SIMD::Set1<W>(1.0f) - t;
            auto linearResult = QuaternionPacket{
                SIMD::FMA(bFlipped.xs, t, a.xs * one_minus_t),
                SIMD::FMA(bFlipped.ys, t, a.ys * one_minus_t),
                SIMD::FMA(bFlipped.zs, t, a.zs * one_minus_t),
                SIMD::FMA(bFlipped.ws, t, a.ws * one_minus_t)
            };
            return Normalize(linearResult);
        }
    };
    template<int W, USize BlockSize>
    [[nodiscard]] inline QuaternionPacket<W> LoadPacketUnaligned(const SoAChunkQuaternion<BlockSize>& soa, USize i) noexcept {
        return { SIMD::Packet<float, W>::LoadUnaligned(&soa.xs[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.ys[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.zs[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.ws[i]) };
    }
    template<int W, USize BlockSize>
    inline void StorePacketUnaligned(SoAChunkQuaternion<BlockSize>& soa, USize i, QuaternionPacket<W> q) noexcept {
        SIMD::Packet<float, W>::StoreUnaligned(&soa.xs[i], q.xs);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.ys[i], q.ys);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.zs[i], q.zs);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.ws[i], q.ws);
    }
    template<int W, USize BlockSize>
    [[nodiscard]] inline QuaternionPacket<W> LoadPacketAligned(const SoAChunkQuaternion<BlockSize>& soa, USize i) noexcept {
        return { SIMD::Packet<float, W>::LoadAligned(&soa.xs[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.ys[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.zs[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.ws[i]) };
    }
    template<int W, USize BlockSize>
    inline void StorePacketAligned(SoAChunkQuaternion<BlockSize>& soa, USize i, QuaternionPacket<W> q) noexcept {
        SIMD::Packet<float, W>::StoreAligned(&soa.xs[i], q.xs);
        SIMD::Packet<float, W>::StoreAligned(&soa.ys[i], q.ys);
        SIMD::Packet<float, W>::StoreAligned(&soa.zs[i], q.zs);
        SIMD::Packet<float, W>::StoreAligned(&soa.ws[i], q.ws);
    }

    template<int W, USize BlockSize>
    [[nodiscard]] inline QuaternionPacket<W> LoadPacketMaskedUnaligned(const SoAChunkQuaternion<BlockSize>& soa, USize i, USize count) noexcept {
        return { SIMD::MaskedLoadUnaligned<W>(&soa.xs[i], count),
                 SIMD::MaskedLoadUnaligned<W>(&soa.ys[i], count),
                 SIMD::MaskedLoadUnaligned<W>(&soa.zs[i], count),
                 SIMD::MaskedLoadUnaligned<W>(&soa.ws[i], count) };
    }

    template<int W, USize BlockSize>
    inline void StorePacketMaskedUnaligned(SoAChunkQuaternion<BlockSize>& soa, USize i, QuaternionPacket<W> q, USize count) noexcept {
        SIMD::MaskedStoreUnaligned<W>(&soa.xs[i], q.xs, count);
        SIMD::MaskedStoreUnaligned<W>(&soa.ys[i], q.ys, count);
        SIMD::MaskedStoreUnaligned<W>(&soa.zs[i], q.zs, count);
        SIMD::MaskedStoreUnaligned<W>(&soa.ws[i], q.ws, count);
    }

    template<int W, USize BlockSize>
    [[nodiscard]] inline QuaternionPacket<W> LoadPacketMaskedAligned(const SoAChunkQuaternion<BlockSize>& soa, USize i, USize count) noexcept {
        return { SIMD::MaskedLoadAligned<W>(&soa.xs[i], count),
                 SIMD::MaskedLoadAligned<W>(&soa.ys[i], count),
                 SIMD::MaskedLoadAligned<W>(&soa.zs[i], count),
                 SIMD::MaskedLoadAligned<W>(&soa.ws[i], count) };
    }

    template<int W, USize BlockSize>
    inline void StorePacketMaskedAligned(SoAChunkQuaternion<BlockSize>& soa, USize i, QuaternionPacket<W> q, USize count) noexcept {
        SIMD::MaskedStoreAligned<W>(&soa.xs[i], q.xs, count);
        SIMD::MaskedStoreAligned<W>(&soa.ys[i], q.ys, count);
        SIMD::MaskedStoreAligned<W>(&soa.zs[i], q.zs, count);
        SIMD::MaskedStoreAligned<W>(&soa.ws[i], q.ws, count);
    }

    template<int W>
    struct MatrixPacket {
        SIMD::Packet<float, W> c0x, c0y, c0z, c0w;
        SIMD::Packet<float, W> c1x, c1y, c1z, c1w;
        SIMD::Packet<float, W> c2x, c2y, c2z, c2w;
        SIMD::Packet<float, W> c3x, c3y, c3z, c3w;
        static inline MatrixPacket Compose(Vector3Packet<W> pos, QuaternionPacket<W> q, Vector3Packet<W> s) noexcept {
            auto two  = SIMD::Set1<W>(2.0f);
            auto one  = SIMD::Set1<W>(1.0f);
            auto zero = pos.xs - pos.xs;
            auto xx = q.xs * q.xs;
            auto yy = q.ys * q.ys;
            auto zz = q.zs * q.zs;
            auto xy = q.xs * q.ys;
            auto xz = q.xs * q.zs;
            auto yz = q.ys * q.zs;
            auto wx = q.ws * q.xs;
            auto wy = q.ws * q.ys;
            auto wz = q.ws * q.zs;
            auto m00 = one - two * (yy + zz);
            auto m01 = two * (xy + wz);
            auto m02 = two * (xz - wy);
            auto m10 = two * (xy - wz);
            auto m11 = one - two * (xx + zz);
            auto m12 = two * (yz + wx);
            auto m20 = two * (xz + wy);
            auto m21 = two * (yz - wx);
            auto m22 = one - two * (xx + yy);
            m00 = m00 * s.xs; m01 = m01 * s.xs; m02 = m02 * s.xs;
            m10 = m10 * s.ys; m11 = m11 * s.ys; m12 = m12 * s.ys;
            m20 = m20 * s.zs; m21 = m21 * s.zs; m22 = m22 * s.zs;
            return { m00, m01, m02, zero,
                     m10, m11, m12, zero,
                     m20, m21, m22, zero,
                     pos.xs, pos.ys, pos.zs, one };
        }
    };
    template<int W, USize BlockSize>
    [[nodiscard]] inline MatrixPacket<W> LoadPacketUnaligned(const SoAChunkMatrix4x4<BlockSize>& soa, USize i) noexcept {
        return { SIMD::Packet<float, W>::LoadUnaligned(&soa.c0x[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c0y[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c0z[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c0w[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c1x[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c1y[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c1z[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c1w[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c2x[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c2y[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c2z[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c2w[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c3x[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c3y[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c3z[i]),
                 SIMD::Packet<float, W>::LoadUnaligned(&soa.c3w[i]) };
    }
    template<int W, USize BlockSize>
    inline void StorePacketUnaligned(SoAChunkMatrix4x4<BlockSize>& soa, USize i, MatrixPacket<W> m) noexcept {
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c0x[i], m.c0x);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c0y[i], m.c0y);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c0z[i], m.c0z);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c0w[i], m.c0w);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c1x[i], m.c1x);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c1y[i], m.c1y);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c1z[i], m.c1z);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c1w[i], m.c1w);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c2x[i], m.c2x);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c2y[i], m.c2y);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c2z[i], m.c2z);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c2w[i], m.c2w);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c3x[i], m.c3x);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c3y[i], m.c3y);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c3z[i], m.c3z);
        SIMD::Packet<float, W>::StoreUnaligned(&soa.c3w[i], m.c3w);
    }
    template<int W, USize BlockSize>
    [[nodiscard]] inline MatrixPacket<W> LoadPacketAligned(const SoAChunkMatrix4x4<BlockSize>& soa, USize i) noexcept {
        return { SIMD::Packet<float, W>::LoadAligned(&soa.c0x[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c0y[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c0z[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c0w[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c1x[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c1y[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c1z[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c1w[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c2x[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c2y[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c2z[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c2w[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c3x[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c3y[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c3z[i]),
                 SIMD::Packet<float, W>::LoadAligned(&soa.c3w[i]) };
    }
    template<int W, USize BlockSize>
    inline void StorePacketAligned(SoAChunkMatrix4x4<BlockSize>& soa, USize i, MatrixPacket<W> m) noexcept {
        SIMD::Packet<float, W>::StoreAligned(&soa.c0x[i], m.c0x);
        SIMD::Packet<float, W>::StoreAligned(&soa.c0y[i], m.c0y);
        SIMD::Packet<float, W>::StoreAligned(&soa.c0z[i], m.c0z);
        SIMD::Packet<float, W>::StoreAligned(&soa.c0w[i], m.c0w);
        SIMD::Packet<float, W>::StoreAligned(&soa.c1x[i], m.c1x);
        SIMD::Packet<float, W>::StoreAligned(&soa.c1y[i], m.c1y);
        SIMD::Packet<float, W>::StoreAligned(&soa.c1z[i], m.c1z);
        SIMD::Packet<float, W>::StoreAligned(&soa.c1w[i], m.c1w);
        SIMD::Packet<float, W>::StoreAligned(&soa.c2x[i], m.c2x);
        SIMD::Packet<float, W>::StoreAligned(&soa.c2y[i], m.c2y);
        SIMD::Packet<float, W>::StoreAligned(&soa.c2z[i], m.c2z);
        SIMD::Packet<float, W>::StoreAligned(&soa.c2w[i], m.c2w);
        SIMD::Packet<float, W>::StoreAligned(&soa.c3x[i], m.c3x);
        SIMD::Packet<float, W>::StoreAligned(&soa.c3y[i], m.c3y);
        SIMD::Packet<float, W>::StoreAligned(&soa.c3z[i], m.c3z);
        SIMD::Packet<float, W>::StoreAligned(&soa.c3w[i], m.c3w);
    }

    template<int W>
    struct DVector3Packet {
        SIMD::Packet<double, W> xs;
        SIMD::Packet<double, W> ys;
        SIMD::Packet<double, W> zs;
        static inline DVector3Packet Add(DVector3Packet a, DVector3Packet b) noexcept {
            return { a.xs + b.xs, a.ys + b.ys, a.zs + b.zs };
        }
        static inline DVector3Packet Sub(DVector3Packet a, DVector3Packet b) noexcept {
            return { a.xs - b.xs, a.ys - b.ys, a.zs - b.zs };
        }
    };

    template<USize BlockSize>
    [[nodiscard]] inline DVector3Packet<4> LoadDVector3PacketUnaligned(const SoAChunkDVector3<BlockSize>& soa, USize i) noexcept {
        return { SIMD::Packet<double, 4>::LoadUnaligned(&soa.xs[i]),
                 SIMD::Packet<double, 4>::LoadUnaligned(&soa.ys[i]),
                 SIMD::Packet<double, 4>::LoadUnaligned(&soa.zs[i]) };
    }

    template<USize BlockSize>
    [[nodiscard]] inline DVector3Packet<2> LoadDVector3PacketUnaligned2(const SoAChunkDVector3<BlockSize>& soa, USize i) noexcept {
        return { SIMD::Packet<double, 2>::LoadUnaligned(&soa.xs[i]),
                 SIMD::Packet<double, 2>::LoadUnaligned(&soa.ys[i]),
                 SIMD::Packet<double, 2>::LoadUnaligned(&soa.zs[i]) };
    }

    template<int FloatW, int DoubleW>
    [[nodiscard]] inline Vector3Packet<FloatW> ConvertDVector3ToVector3(DVector3Packet<DoubleW> d) noexcept {
        static_assert(FloatW == DoubleW || (FloatW == 4 && DoubleW == 4) || (FloatW == 2 && DoubleW == 2) || (FloatW == 1 && DoubleW == 1));
        return { SIMD::ConvertDoubleToFloat(d.xs),
                 SIMD::ConvertDoubleToFloat(d.ys),
                 SIMD::ConvertDoubleToFloat(d.zs) };
    }
}