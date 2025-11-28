module;
export module Cap.Math:MatrixOps;
import Language;
import SIMD;
import :Storage;
import :Compute;
import <cmath>;
export namespace Cap {
    template<int W>
    [[nodiscard]] inline MatrixPacket<W> Transpose(MatrixPacket<W> m) noexcept {
        return { m.c0x, m.c1x, m.c2x, m.c3x,
                 m.c0y, m.c1y, m.c2y, m.c3y,
                 m.c0z, m.c1z, m.c2z, m.c3z,
                 m.c0w, m.c1w, m.c2w, m.c3w };
    }

    template<int W>
    [[nodiscard]] inline MatrixPacket<W> InverseAffine(MatrixPacket<W> m) noexcept {
        Vector3Packet<W> r0{ m.c0x, m.c0y, m.c0z };
        Vector3Packet<W> r1{ m.c1x, m.c1y, m.c1z };
        Vector3Packet<W> r2{ m.c2x, m.c2y, m.c2z };
        auto c0 = Vector3Packet<W>::Cross(r1, r2);
        auto c1 = Vector3Packet<W>::Cross(r2, r0);
        auto c2 = Vector3Packet<W>::Cross(r0, r1);
        auto det = Vector3Packet<W>::Dot(r0, c0);
        auto invDet = SIMD::Recp(det);
        c0 = Vector3Packet<W>::Mul(c0, invDet);
        c1 = Vector3Packet<W>::Mul(c1, invDet);
        c2 = Vector3Packet<W>::Mul(c2, invDet);
        Vector3Packet<W> t{ m.c3x, m.c3y, m.c3z };
        auto tx = SIMD::Set1<W>(-1.0f) * Vector3Packet<W>::Dot(c0, t);
        auto ty = SIMD::Set1<W>(-1.0f) * Vector3Packet<W>::Dot(c1, t);
        auto tz = SIMD::Set1<W>(-1.0f) * Vector3Packet<W>::Dot(c2, t);
        auto zero = SIMD::Set1<W>(0.0f);
        auto one  = SIMD::Set1<W>(1.0f);
        return { c0.xs, c0.ys, c0.zs, zero,
                 c1.xs, c1.ys, c1.zs, zero,
                 c2.xs, c2.ys, c2.zs, zero,
                 tx, ty, tz, one };
    }

    template<int W>
    [[nodiscard]] inline MatrixPacket<W> LookAt(Vector3Packet<W> eye, Vector3Packet<W> target, Vector3Packet<W> up) noexcept {
        auto f = Vector3Packet<W>::Normalize(Vector3Packet<W>::Sub(target, eye));
        auto r = Vector3Packet<W>::Normalize(Vector3Packet<W>::Cross(f, up));
        auto u = Vector3Packet<W>::Cross(r, f);
        auto tx = SIMD::Set1<W>(-1.0f) * Vector3Packet<W>::Dot(r, eye);
        auto ty = SIMD::Set1<W>(-1.0f) * Vector3Packet<W>::Dot(u, eye);
        auto tz = SIMD::Set1<W>(1.0f) * Vector3Packet<W>::Dot(f, eye);
        auto zero = SIMD::Set1<W>(0.0f);
        auto one  = SIMD::Set1<W>(1.0f);
        return { r.xs, u.xs, SIMD::Set1<W>(-1.0f) * f.xs, zero,
                 r.ys, u.ys, SIMD::Set1<W>(-1.0f) * f.ys, zero,
                 r.zs, u.zs, SIMD::Set1<W>(-1.0f) * f.zs, zero,
                 tx, ty, tz, one };
    }

    template<int W>
    [[nodiscard]] inline MatrixPacket<W> Multiply(MatrixPacket<W> a, MatrixPacket<W> b) noexcept {
        MatrixPacket<W> r;
        r.c0x = SIMD::FMA(a.c0x, b.c0x, SIMD::FMA(a.c1x, b.c0y, SIMD::FMA(a.c2x, b.c0z, a.c3x * b.c0w)));
        r.c0y = SIMD::FMA(a.c0y, b.c0x, SIMD::FMA(a.c1y, b.c0y, SIMD::FMA(a.c2y, b.c0z, a.c3y * b.c0w)));
        r.c0z = SIMD::FMA(a.c0z, b.c0x, SIMD::FMA(a.c1z, b.c0y, SIMD::FMA(a.c2z, b.c0z, a.c3z * b.c0w)));
        r.c0w = SIMD::FMA(a.c0w, b.c0x, SIMD::FMA(a.c1w, b.c0y, SIMD::FMA(a.c2w, b.c0z, a.c3w * b.c0w)));

        r.c1x = SIMD::FMA(a.c0x, b.c1x, SIMD::FMA(a.c1x, b.c1y, SIMD::FMA(a.c2x, b.c1z, a.c3x * b.c1w)));
        r.c1y = SIMD::FMA(a.c0y, b.c1x, SIMD::FMA(a.c1y, b.c1y, SIMD::FMA(a.c2y, b.c1z, a.c3y * b.c1w)));
        r.c1z = SIMD::FMA(a.c0z, b.c1x, SIMD::FMA(a.c1z, b.c1y, SIMD::FMA(a.c2z, b.c1z, a.c3z * b.c1w)));
        r.c1w = SIMD::FMA(a.c0w, b.c1x, SIMD::FMA(a.c1w, b.c1y, SIMD::FMA(a.c2w, b.c1z, a.c3w * b.c1w)));

        r.c2x = SIMD::FMA(a.c0x, b.c2x, SIMD::FMA(a.c1x, b.c2y, SIMD::FMA(a.c2x, b.c2z, a.c3x * b.c2w)));
        r.c2y = SIMD::FMA(a.c0y, b.c2x, SIMD::FMA(a.c1y, b.c2y, SIMD::FMA(a.c2y, b.c2z, a.c3y * b.c2w)));
        r.c2z = SIMD::FMA(a.c0z, b.c2x, SIMD::FMA(a.c1z, b.c2y, SIMD::FMA(a.c2z, b.c2z, a.c3z * b.c2w)));
        r.c2w = SIMD::FMA(a.c0w, b.c2x, SIMD::FMA(a.c1w, b.c2y, SIMD::FMA(a.c2w, b.c2z, a.c3w * b.c2w)));

        r.c3x = SIMD::FMA(a.c0x, b.c3x, SIMD::FMA(a.c1x, b.c3y, SIMD::FMA(a.c2x, b.c3z, a.c3x * b.c3w)));
        r.c3y = SIMD::FMA(a.c0y, b.c3x, SIMD::FMA(a.c1y, b.c3y, SIMD::FMA(a.c2y, b.c3z, a.c3y * b.c3w)));
        r.c3z = SIMD::FMA(a.c0z, b.c3x, SIMD::FMA(a.c1z, b.c3y, SIMD::FMA(a.c2z, b.c3z, a.c3z * b.c3w)));
        r.c3w = SIMD::FMA(a.c0w, b.c3x, SIMD::FMA(a.c1w, b.c3y, SIMD::FMA(a.c2w, b.c3z, a.c3w * b.c3w)));
        return r;
    }

    template<int W>
    [[nodiscard]] inline MatrixPacket<W> Inverse(MatrixPacket<W> m) noexcept {
        auto a00 = m.c0x, a01 = m.c1x, a02 = m.c2x, a03 = m.c3x;
        auto a10 = m.c0y, a11 = m.c1y, a12 = m.c2y, a13 = m.c3y;
        auto a20 = m.c0z, a21 = m.c1z, a22 = m.c2z, a23 = m.c3z;
        auto a30 = m.c0w, a31 = m.c1w, a32 = m.c2w, a33 = m.c3w;

        auto b00 = a00 * a11 - a01 * a10;
        auto b01 = a00 * a12 - a02 * a10;
        auto b02 = a00 * a13 - a03 * a10;
        auto b03 = a01 * a12 - a02 * a11;
        auto b04 = a01 * a13 - a03 * a11;
        auto b05 = a02 * a13 - a03 * a12;
        auto b06 = a20 * a31 - a21 * a30;
        auto b07 = a20 * a32 - a22 * a30;
        auto b08 = a20 * a33 - a23 * a30;
        auto b09 = a21 * a32 - a22 * a31;
        auto b10 = a21 * a33 - a23 * a31;
        auto b11 = a22 * a33 - a23 * a32;

        auto det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
        auto invDet = SIMD::Set1<W>(1.0f) / det;

        return {
            (a11 * b11 - a12 * b10 + a13 * b09) * invDet,
            (a02 * b10 - a01 * b11 - a03 * b09) * invDet,
            (a31 * b05 - a32 * b04 + a33 * b03) * invDet,
            (a22 * b04 - a21 * b05 - a23 * b03) * invDet,

            (a12 * b08 - a10 * b11 - a13 * b07) * invDet,
            (a00 * b11 - a02 * b08 + a03 * b07) * invDet,
            (a32 * b02 - a30 * b05 - a33 * b01) * invDet,
            (a20 * b05 - a22 * b02 + a23 * b01) * invDet,

            (a10 * b10 - a11 * b08 + a13 * b06) * invDet,
            (a01 * b08 - a00 * b10 - a03 * b06) * invDet,
            (a30 * b04 - a31 * b02 + a33 * b00) * invDet,
            (a21 * b02 - a20 * b04 - a23 * b00) * invDet,

            (a11 * b07 - a10 * b09 - a12 * b06) * invDet,
            (a00 * b09 - a01 * b07 + a02 * b06) * invDet,
            (a31 * b01 - a30 * b03 - a32 * b00) * invDet,
            (a20 * b03 - a21 * b01 + a22 * b00) * invDet
        };
    }

    template<int W>
    [[nodiscard]] inline MatrixPacket<W> Perspective(float fovY, float aspect, float nearZ, float farZ) noexcept {
        float fScalar = 1.0f / std::tan(fovY * 0.5f);
        float nfScalar = 1.0f / (nearZ - farZ);
        auto zero = SIMD::Set1<W>(0.0f);
        auto one  = SIMD::Set1<W>(1.0f);
        auto f    = SIMD::Set1<W>(fScalar);
        auto invA = SIMD::Set1<W>(1.0f / aspect);
        auto m22  = SIMD::Set1<W>((farZ + nearZ) * nfScalar);
        auto m32  = SIMD::Set1<W>(2.0f * farZ * nearZ * nfScalar);
        auto negOne = SIMD::Set1<W>(-1.0f);
        return { f * invA, zero,   zero,   zero,
                 zero,     f,      zero,   zero,
                 zero,     zero,   m22,    negOne,
                 zero,     zero,   m32,    zero };
    }

    template<int W>
    [[nodiscard]] inline MatrixPacket<W> Orthographic(float left, float right, float bottom, float top, float nearZ, float farZ) noexcept {
        float rl = right - left;
        float tb = top - bottom;
        float fn = farZ - nearZ;
        auto two   = SIMD::Set1<W>(2.0f);
        auto zero  = SIMD::Set1<W>(0.0f);
        auto one   = SIMD::Set1<W>(1.0f);
        auto sx = SIMD::Set1<W>(2.0f / rl);
        auto sy = SIMD::Set1<W>(2.0f / tb);
        auto sz = SIMD::Set1<W>(-2.0f / fn);
        auto tx = SIMD::Set1<W>(-(right + left) / rl);
        auto ty = SIMD::Set1<W>(-(top + bottom) / tb);
        auto tz = SIMD::Set1<W>(-(farZ + nearZ) / fn);
        return { sx,   zero, zero, zero,
                 zero, sy,   zero, zero,
                 zero, zero, sz,   zero,
                 tx,   ty,   tz,   one };
    }
}

export namespace Cap {
    [[nodiscard]] inline Matrix4x4 Transpose(const Matrix4x4& m) noexcept {
        Matrix4x4 r{};
        r.m[0]  = m.m[0];  r.m[1]  = m.m[4];  r.m[2]  = m.m[8];  r.m[3]  = m.m[12];
        r.m[4]  = m.m[1];  r.m[5]  = m.m[5];  r.m[6]  = m.m[9];  r.m[7]  = m.m[13];
        r.m[8]  = m.m[2];  r.m[9]  = m.m[6];  r.m[10] = m.m[10]; r.m[11] = m.m[14];
        r.m[12] = m.m[3];  r.m[13] = m.m[7];  r.m[14] = m.m[11]; r.m[15] = m.m[15];
        return r;
    }

    [[nodiscard]] inline Matrix4x4 InverseAffine(const Matrix4x4& m) noexcept {
        Scalar m00 = m.m[0],  m01 = m.m[1],  m02 = m.m[2];
        Scalar m10 = m.m[4],  m11 = m.m[5],  m12 = m.m[6];
        Scalar m20 = m.m[8],  m21 = m.m[9],  m22 = m.m[10];
        Scalar tx  = m.m[12], ty  = m.m[13], tz  = m.m[14];
        Scalar c00 = m11 * m22 - m12 * m21;
        Scalar c01 = m12 * m20 - m10 * m22;
        Scalar c02 = m10 * m21 - m11 * m20;
        Scalar c10 = m02 * m21 - m01 * m22;
        Scalar c11 = m00 * m22 - m02 * m20;
        Scalar c12 = m01 * m20 - m00 * m21;
        Scalar c20 = m01 * m12 - m02 * m11;
        Scalar c21 = m02 * m10 - m00 * m12;
        Scalar c22 = m00 * m11 - m01 * m10;
        Scalar det = m00 * c00 + m10 * c10 + m20 * c20;
        Scalar invDet = 1.0f / det;
        c00 *= invDet; c01 *= invDet; c02 *= invDet;
        c10 *= invDet; c11 *= invDet; c12 *= invDet;
        c20 *= invDet; c21 *= invDet; c22 *= invDet;
        Scalar itx = -(c00 * tx + c01 * ty + c02 * tz);
        Scalar ity = -(c10 * tx + c11 * ty + c12 * tz);
        Scalar itz = -(c20 * tx + c21 * ty + c22 * tz);
        Matrix4x4 r{};
        r.m[0] = c00; r.m[1] = c10; r.m[2] = c20; r.m[3] = 0.0f;
        r.m[4] = c01; r.m[5] = c11; r.m[6] = c21; r.m[7] = 0.0f;
        r.m[8] = c02; r.m[9] = c12; r.m[10] = c22; r.m[11] = 0.0f;
        r.m[12] = itx; r.m[13] = ity; r.m[14] = itz; r.m[15] = 1.0f;
        return r;
    }

    [[nodiscard]] inline Matrix4x4 LookAt(const Vector3& eye, const Vector3& target, const Vector3& up) noexcept {
        Scalar fx = target.x - eye.x;
        Scalar fy = target.y - eye.y;
        Scalar fz = target.z - eye.z;
        Scalar fl = std::sqrt(fx*fx + fy*fy + fz*fz);
        fx /= fl; fy /= fl; fz /= fl;
        Scalar rx = fy*up.z - fz*up.y;
        Scalar ry = fz*up.x - fx*up.z;
        Scalar rz = fx*up.y - fy*up.x;
        Scalar rl = std::sqrt(rx*rx + ry*ry + rz*rz);
        rx /= rl; ry /= rl; rz /= rl;
        Scalar ux = ry*fz - rz*fy;
        Scalar uy = rz*fx - rx*fz;
        Scalar uz = rx*fy - ry*fx;
        Scalar tx = -(rx*eye.x + ry*eye.y + rz*eye.z);
        Scalar ty = -(ux*eye.x + uy*eye.y + uz*eye.z);
        Scalar tz = (fx*eye.x + fy*eye.y + fz*eye.z);
        Matrix4x4 m{};
        m.m[0] = rx; m.m[1] = ux; m.m[2] = -fx; m.m[3] = 0.0f;
        m.m[4] = ry; m.m[5] = uy; m.m[6] = -fy; m.m[7] = 0.0f;
        m.m[8] = rz; m.m[9] = uz; m.m[10] = -fz; m.m[11] = 0.0f;
        m.m[12] = tx; m.m[13] = ty; m.m[14] = tz; m.m[15] = 1.0f;
        return m;
    }

    [[nodiscard]] inline Matrix4x4 Perspective(Scalar fov, Scalar aspect, Scalar znear, Scalar zfar) noexcept {
        Scalar f = 1.0f / std::tan(fov * 0.5f);
        Scalar A = (zfar + znear) / (znear - zfar);
        Scalar B = (2.0f * zfar * znear) / (znear - zfar);
        Matrix4x4 m{};
        m.m[0] = f / aspect; m.m[1] = 0.0f; m.m[2] = 0.0f; m.m[3] = 0.0f;
        m.m[4] = 0.0f; m.m[5] = f; m.m[6] = 0.0f; m.m[7] = 0.0f;
        m.m[8] = 0.0f; m.m[9] = 0.0f; m.m[10] = A; m.m[11] = -1.0f;
        m.m[12] = 0.0f; m.m[13] = 0.0f; m.m[14] = B; m.m[15] = 0.0f;
        return m;
    }

    [[nodiscard]] inline Matrix4x4 Orthographic(Scalar left, Scalar right, Scalar bottom, Scalar top, Scalar znear, Scalar zfar) noexcept {
        Scalar rl = right - left;
        Scalar tb = top - bottom;
        Scalar fn = zfar - znear;
        Matrix4x4 m{};
        m.m[0] = 2.0f/rl; m.m[1] = 0.0f;    m.m[2] = 0.0f;     m.m[3] = 0.0f;
        m.m[4] = 0.0f;    m.m[5] = 2.0f/tb; m.m[6] = 0.0f;     m.m[7] = 0.0f;
        m.m[8] = 0.0f;    m.m[9] = 0.0f;    m.m[10] = -2.0f/fn; m.m[11] = 0.0f;
        m.m[12] = -(right+left)/rl; m.m[13] = -(top+bottom)/tb; m.m[14] = -(zfar+znear)/fn; m.m[15] = 1.0f;
        return m;
    }
}
