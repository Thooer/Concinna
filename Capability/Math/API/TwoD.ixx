module;
export module Cap.Math:TwoD;
import Language;
import :Storage;
export namespace Cap {
    struct Vector2 { Scalar x; Scalar y; };
    struct Point2i { Int32 x; Int32 y; };
    struct Rect { Scalar x; Scalar y; Scalar w; Scalar h; };
    struct Matrix3x2 { Scalar m00; Scalar m01; Scalar m10; Scalar m11; Scalar m20; Scalar m21; };
    [[nodiscard]] inline bool Contains(const Rect& r, const Vector2& p) noexcept {
        return p.x >= r.x && p.x <= r.x + r.w && p.y >= r.y && p.y <= r.y + r.h;
    }
    [[nodiscard]] inline Rect Inflate(const Rect& r, Scalar dx, Scalar dy) noexcept {
        return { r.x - dx, r.y - dy, r.w + 2.0f * dx, r.h + 2.0f * dy };
    }
    [[nodiscard]] inline Rect Union(const Rect& a, const Rect& b) noexcept {
        Scalar x = a.x < b.x ? a.x : b.x;
        Scalar y = a.y < b.y ? a.y : b.y;
        Scalar r1 = a.x + a.w; Scalar r2 = b.x + b.w;
        Scalar b1 = a.y + a.h; Scalar b2 = b.y + b.h;
        Scalar w = (r1 > r2 ? r1 : r2) - x;
        Scalar h = (b1 > b2 ? b1 : b2) - y;
        return { x, y, w, h };
    }
    [[nodiscard]] inline Rect Intersect(const Rect& a, const Rect& b) noexcept {
        Scalar x1 = a.x > b.x ? a.x : b.x;
        Scalar y1 = a.y > b.y ? a.y : b.y;
        Scalar x2 = (a.x + a.w) < (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
        Scalar y2 = (a.y + a.h) < (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
        Scalar w = x2 - x1; Scalar h = y2 - y1;
        if (w < 0.0f || h < 0.0f) return { 0, 0, 0, 0 };
        return { x1, y1, w, h };
    }
    [[nodiscard]] inline Matrix3x2 Translation(Scalar tx, Scalar ty) noexcept { return { 1, 0, 0, 1, tx, ty }; }
    [[nodiscard]] inline Matrix3x2 Scale(Scalar sx, Scalar sy) noexcept { return { sx, 0, 0, sy, 0, 0 }; }
    [[nodiscard]] inline Matrix3x2 Rotation(Scalar c, Scalar s) noexcept { return { c, s, -s, c, 0, 0 }; }
    [[nodiscard]] inline Matrix3x2 Multiply(const Matrix3x2& a, const Matrix3x2& b) noexcept {
        Matrix3x2 r{};
        r.m00 = a.m00*b.m00 + a.m10*b.m01;
        r.m01 = a.m01*b.m00 + a.m11*b.m01;
        r.m10 = a.m00*b.m10 + a.m10*b.m11;
        r.m11 = a.m01*b.m10 + a.m11*b.m11;
        r.m20 = a.m00*b.m20 + a.m10*b.m21 + a.m20;
        r.m21 = a.m01*b.m20 + a.m11*b.m21 + a.m21;
        return r;
    }
    [[nodiscard]] inline Vector2 TransformPoint(const Matrix3x2& m, const Vector2& p) noexcept {
        return { m.m00*p.x + m.m10*p.y + m.m20, m.m01*p.x + m.m11*p.y + m.m21 };
    }
}

