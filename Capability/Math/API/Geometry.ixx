module;
export module Cap.Math:Geometry;
import Language;
import SIMD;
import :Storage;
import :Compute;
import <cmath>;
export namespace Cap {
    struct Plane { Vector3 n; Scalar d; };
    struct Sphere { Vector3 c; Scalar r; };
    struct AABB { Vector3 min_; Vector3 max_; };
    struct Ray { Vector3 o; Vector3 d; };
    struct Frustum { Plane planes[6]; };

    [[nodiscard]] inline bool IntersectRayPlane(const Ray& r, const Plane& p, Scalar& t) noexcept {
        Scalar denom = p.n.x * r.d.x + p.n.y * r.d.y + p.n.z * r.d.z;
        if (std::fabs(denom) < 1e-6f) return false;
        t = -(p.n.x * r.o.x + p.n.y * r.o.y + p.n.z * r.o.z + p.d) / denom;
        return t >= 0.0f;
    }
    [[nodiscard]] inline bool IntersectRaySphere(const Ray& r, const Sphere& s, Scalar& t) noexcept {
        Scalar ox = r.o.x - s.c.x;
        Scalar oy = r.o.y - s.c.y;
        Scalar oz = r.o.z - s.c.z;
        Scalar b = ox * r.d.x + oy * r.d.y + oz * r.d.z;
        Scalar c = ox*ox + oy*oy + oz*oz - s.r * s.r;
        Scalar disc = b*b - c;
        if (disc < 0.0f) return false;
        Scalar sd = std::sqrt(disc);
        t = -b - sd;
        if (t < 0.0f) t = -b + sd;
        return t >= 0.0f;
    }
    [[nodiscard]] inline bool IntersectRayAABB(const Ray& r, const AABB& b, Scalar& tmin, Scalar& tmax) noexcept {
        Scalar invx = 1.0f / r.d.x;
        Scalar tx1 = (b.min_.x - r.o.x) * invx;
        Scalar tx2 = (b.max_.x - r.o.x) * invx;
        tmin = std::fmin(tx1, tx2);
        tmax = std::fmax(tx1, tx2);
        Scalar invy = 1.0f / r.d.y;
        Scalar ty1 = (b.min_.y - r.o.y) * invy;
        Scalar ty2 = (b.max_.y - r.o.y) * invy;
        tmin = std::fmax(tmin, std::fmin(ty1, ty2));
        tmax = std::fmin(tmax, std::fmax(ty1, ty2));
        Scalar invz = 1.0f / r.d.z;
        Scalar tz1 = (b.min_.z - r.o.z) * invz;
        Scalar tz2 = (b.max_.z - r.o.z) * invz;
        tmin = std::fmax(tmin, std::fmin(tz1, tz2));
        tmax = std::fmin(tmax, std::fmax(tz1, tz2));
        return tmax >= tmin && tmax >= 0.0f;
    }
    [[nodiscard]] inline bool IntersectRayTriangle(const Ray& r, const Vector3& v0, const Vector3& v1, const Vector3& v2, Scalar& t) noexcept {
        Vector3 e1{ v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
        Vector3 e2{ v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };
        Vector3 p{ r.d.y * e2.z - r.d.z * e2.y, r.d.z * e2.x - r.d.x * e2.z, r.d.x * e2.y - r.d.y * e2.x };
        Scalar det = e1.x * p.x + e1.y * p.y + e1.z * p.z;
        if (std::fabs(det) < 1e-8f) return false;
        Scalar invDet = 1.0f / det;
        Vector3 tvec{ r.o.x - v0.x, r.o.y - v0.y, r.o.z - v0.z };
        Scalar u = (tvec.x * p.x + tvec.y * p.y + tvec.z * p.z) * invDet;
        if (u < 0.0f || u > 1.0f) return false;
        Vector3 q{ tvec.y * e1.z - tvec.z * e1.y, tvec.z * e1.x - tvec.x * e1.z, tvec.x * e1.y - tvec.y * e1.x };
        Scalar v = (r.d.x * q.x + r.d.y * q.y + r.d.z * q.z) * invDet;
        if (v < 0.0f || u + v > 1.0f) return false;
        t = (e2.x * q.x + e2.y * q.y + e2.z * q.z) * invDet;
        return t >= 0.0f;
    }

    template<int W>
    struct PlanePacket {
        SIMD::Packet<float, W> nx;
        SIMD::Packet<float, W> ny;
        SIMD::Packet<float, W> nz;
        SIMD::Packet<float, W> d;
        static inline SIMD::Packet<float, W> Distance(Vector3Packet<W> p) noexcept {
            auto r = nx * p.xs;
            r = SIMD::FMA(ny, p.ys, r);
            r = SIMD::FMA(nz, p.zs, r);
            return r + d;
        }
    };

    template<int W>
    struct RayPacket {
        Vector3Packet<W> o;
        Vector3Packet<W> d;
    };

    template<int W>
    struct AABBPacket {
        Vector3Packet<W> min_;
        Vector3Packet<W> max_;
        static inline AABBPacket Merge(AABBPacket a, AABBPacket b) noexcept {
            return { Vector3Packet<W>::Min(a.min_, b.min_), Vector3Packet<W>::Max(a.max_, b.max_) };
        }
        static inline Vector3Packet<W> Center(AABBPacket b) noexcept {
            auto h = SIMD::Set1<W>(0.5f);
            return Vector3Packet<W>::Mul(Vector3Packet<W>::Add(b.min_, b.max_), h);
        }
        static inline Vector3Packet<W> Extents(AABBPacket b) noexcept {
            auto h = SIMD::Set1<W>(0.5f);
            return Vector3Packet<W>::Mul(Vector3Packet<W>::Sub(b.max_, b.min_), h);
        }
        static inline SIMD::PacketMask<W> Contains(AABBPacket box, Vector3Packet<W> point) noexcept {
            auto one  = SIMD::Set1<W>(1.0f);
            auto zero = SIMD::Set1<W>(0.0f);
            auto f = SIMD::Select(SIMD::GreaterEqual(point.xs, box.min_.xs), one, zero);
            f = SIMD::Select(SIMD::LessEqual(point.xs, box.max_.xs), f, zero);
            f = SIMD::Select(SIMD::GreaterEqual(point.ys, box.min_.ys), f, zero);
            f = SIMD::Select(SIMD::LessEqual(point.ys, box.max_.ys), f, zero);
            f = SIMD::Select(SIMD::GreaterEqual(point.zs, box.min_.zs), f, zero);
            f = SIMD::Select(SIMD::LessEqual(point.zs, box.max_.zs), f, zero);
            return SIMD::Equal(f, one);
        }
    };

    template<int W>
    struct FrustumPacket {
        PlanePacket<W> planes[6];
        static inline SIMD::PacketMask<W> Intersects(FrustumPacket f, AABBPacket<W> box) noexcept {
            auto c = AABBPacket<W>::Center(box);
            auto e = AABBPacket<W>::Extents(box);
            auto one  = SIMD::Set1<W>(1.0f);
            auto zero = SIMD::Set1<W>(0.0f);
            auto ok = one;
            {
                auto dist = f.planes[0].Distance(c);
                auto r = SIMD::Abs(f.planes[0].nx) * e.xs;
                r = SIMD::FMA(SIMD::Abs(f.planes[0].ny), e.ys, r);
                r = SIMD::FMA(SIMD::Abs(f.planes[0].nz), e.zs, r);
                ok = SIMD::And(ok, SIMD::Select(SIMD::GreaterEqual(dist + r, zero), one, zero));
            }
            {
                auto dist = f.planes[1].Distance(c);
                auto r = SIMD::Abs(f.planes[1].nx) * e.xs;
                r = SIMD::FMA(SIMD::Abs(f.planes[1].ny), e.ys, r);
                r = SIMD::FMA(SIMD::Abs(f.planes[1].nz), e.zs, r);
                ok = SIMD::And(ok, SIMD::Select(SIMD::GreaterEqual(dist + r, zero), one, zero));
            }
            {
                auto dist = f.planes[2].Distance(c);
                auto r = SIMD::Abs(f.planes[2].nx) * e.xs;
                r = SIMD::FMA(SIMD::Abs(f.planes[2].ny), e.ys, r);
                r = SIMD::FMA(SIMD::Abs(f.planes[2].nz), e.zs, r);
                ok = SIMD::And(ok, SIMD::Select(SIMD::GreaterEqual(dist + r, zero), one, zero));
            }
            {
                auto dist = f.planes[3].Distance(c);
                auto r = SIMD::Abs(f.planes[3].nx) * e.xs;
                r = SIMD::FMA(SIMD::Abs(f.planes[3].ny), e.ys, r);
                r = SIMD::FMA(SIMD::Abs(f.planes[3].nz), e.zs, r);
                ok = SIMD::And(ok, SIMD::Select(SIMD::GreaterEqual(dist + r, zero), one, zero));
            }
            {
                auto dist = f.planes[4].Distance(c);
                auto r = SIMD::Abs(f.planes[4].nx) * e.xs;
                r = SIMD::FMA(SIMD::Abs(f.planes[4].ny), e.ys, r);
                r = SIMD::FMA(SIMD::Abs(f.planes[4].nz), e.zs, r);
                ok = SIMD::And(ok, SIMD::Select(SIMD::GreaterEqual(dist + r, zero), one, zero));
            }
            {
                auto dist = f.planes[5].Distance(c);
                auto r = SIMD::Abs(f.planes[5].nx) * e.xs;
                r = SIMD::FMA(SIMD::Abs(f.planes[5].ny), e.ys, r);
                r = SIMD::FMA(SIMD::Abs(f.planes[5].nz), e.zs, r);
                ok = SIMD::And(ok, SIMD::Select(SIMD::GreaterEqual(dist + r, zero), one, zero));
            }
            return SIMD::Equal(ok, one);
        }
    };
}
