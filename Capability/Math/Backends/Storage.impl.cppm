module Math:Storage;
import :Storage;
import Language;
namespace Math {
    [[nodiscard]] Vector3 ToFloat(const DVector3& d) noexcept {
        return Vector3{ static_cast<Scalar>(d.x), static_cast<Scalar>(d.y), static_cast<Scalar>(d.z) };
    }
    [[nodiscard]] Scalar HalfToScalar(HalfT h) noexcept {
        UInt16 hb = h.bits;
        UInt32 s = static_cast<UInt32>(hb & 0x8000u) << 16;
        UInt32 e = static_cast<UInt32>(hb & 0x7C00u);
        UInt32 m = static_cast<UInt32>(hb & 0x03FFu);
        UInt32 fb;
        if (e == 0) {
            if (m == 0) {
                fb = s;
            } else {
                UInt32 mm = m;
                int exp = -1;
                while ((mm & 0x0400u) == 0u) { mm <<= 1; --exp; }
                mm &= 0x03FFu;
                UInt32 fe = static_cast<UInt32>((127 - 15 + exp) << 23);
                fb = s | fe | (mm << 13);
            }
        } else if (e == 0x7C00u) {
            fb = s | 0x7F800000u | (m << 13);
        } else {
            UInt32 fe = static_cast<UInt32>(((e >> 10) - 15 + 127) << 23);
            fb = s | fe | (m << 13);
        }
        union { UInt32 u; Scalar f; } cvt{ fb };
        return cvt.f;
    }
    [[nodiscard]] HalfT ScalarToHalf(Scalar f) noexcept {
        union { Scalar f; UInt32 u; } cvt{ f };
        UInt32 u = cvt.u;
        UInt32 s = (u >> 16) & 0x8000u;
        Int32 e = static_cast<Int32>((u >> 23) & 0xFFu) - 127 + 15;
        UInt32 m = u & 0x007FFFFFu;
        UInt16 h;
        if (e <= 0) {
            if (e < -10) {
                h = static_cast<UInt16>(s);
            } else {
                m |= 0x00800000u;
                UInt32 t = m >> static_cast<UInt32>(1 - e);
                UInt32 r = t + 0x00001000u;
                h = static_cast<UInt16>(s | (r >> 13));
            }
        } else if (e >= 31) {
            h = static_cast<UInt16>(s | 0x7C00u);
        } else {
            UInt32 r = m + 0x00001000u;
            h = static_cast<UInt16>(s | (static_cast<UInt32>(e) << 10) | (r >> 13));
        }
        return HalfT{ h };
    }
}
