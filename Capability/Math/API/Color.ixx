module;
export module Cap.Math:Color;
import Lang;
import :Storage;
import <cmath>;
export namespace Cap {
    [[nodiscard]] inline Vector3 RGBToHSV(const Vector3& rgb) noexcept {
        Scalar r = rgb.x, g = rgb.y, b = rgb.z;
        Scalar mx = std::fmax(r, std::fmax(g, b));
        Scalar mn = std::fmin(r, std::fmin(g, b));
        Scalar d = mx - mn;
        Scalar h = 0.0f;
        if (d > 0.0f) {
            if (mx == r) h = (g - b) / d + (g < b ? 6.0f : 0.0f);
            else if (mx == g) h = (b - r) / d + 2.0f;
            else h = (r - g) / d + 4.0f;
            h /= 6.0f;
        }
        Scalar s = mx == 0.0f ? 0.0f : d / mx;
        Scalar v = mx;
        return { h, s, v };
    }
    [[nodiscard]] inline Vector3 HSVToRGB(const Vector3& hsv) noexcept {
        Scalar h = hsv.x, s = hsv.y, v = hsv.z;
        Scalar c = v * s;
        Scalar x = c * (1.0f - std::fabs(std::fmod(h * 6.0f, 2.0f) - 1.0f));
        Scalar m = v - c;
        Scalar r = 0, g = 0, b = 0;
        Scalar h6 = h * 6.0f;
        if (h6 < 1.0f) { r = c; g = x; }
        else if (h6 < 2.0f) { r = x; g = c; }
        else if (h6 < 3.0f) { g = c; b = x; }
        else if (h6 < 4.0f) { g = x; b = c; }
        else if (h6 < 5.0f) { r = x; b = c; }
        else { r = c; b = x; }
        return { r + m, g + m, b + m };
    }
    [[nodiscard]] inline Vector3 RGBToHSL(const Vector3& rgb) noexcept {
        Scalar r = rgb.x, g = rgb.y, b = rgb.z;
        Scalar mx = std::fmax(r, std::fmax(g, b));
        Scalar mn = std::fmin(r, std::fmin(g, b));
        Scalar l = (mx + mn) * 0.5f;
        Scalar h = 0.0f, s = 0.0f;
        Scalar d = mx - mn;
        if (d > 0.0f) {
            s = l > 0.5f ? d / (2.0f - mx - mn) : d / (mx + mn);
            if (mx == r) h = (g - b) / d + (g < b ? 6.0f : 0.0f);
            else if (mx == g) h = (b - r) / d + 2.0f;
            else h = (r - g) / d + 4.0f;
            h /= 6.0f;
        }
        return { h, s, l };
    }
    [[nodiscard]] inline Vector3 HSLToRGB(const Vector3& hsl) noexcept {
        auto hue2rgb = [](Scalar p, Scalar q, Scalar t) noexcept -> Scalar {
            if (t < 0.0f) t += 1.0f;
            if (t > 1.0f) t -= 1.0f;
            if (t < 1.0f/6.0f) return p + (q - p) * 6.0f * t;
            if (t < 1.0f/2.0f) return q;
            if (t < 2.0f/3.0f) return p + (q - p) * (2.0f/3.0f - t) * 6.0f;
            return p;
        };
        Scalar h = hsl.x, s = hsl.y, l = hsl.z;
        Scalar r, g, b;
        if (s == 0.0f) { r = g = b = l; }
        else {
            Scalar q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
            Scalar p = 2.0f * l - q;
            r = hue2rgb(p, q, h + 1.0f/3.0f);
            g = hue2rgb(p, q, h);
            b = hue2rgb(p, q, h - 1.0f/3.0f);
        }
        return { r, g, b };
    }
    [[nodiscard]] inline Vector3 LinearToSRGB(const Vector3& c) noexcept {
        auto f = [](Scalar x) noexcept -> Scalar {
            if (x <= 0.0031308f) return 12.92f * x;
            return 1.055f * std::pow(x, 1.0f/2.4f) - 0.055f;
        };
        return { f(c.x), f(c.y), f(c.z) };
    }
    [[nodiscard]] inline Vector3 SRGBToLinear(const Vector3& c) noexcept {
        auto f = [](Scalar x) noexcept -> Scalar {
            if (x <= 0.04045f) return x / 12.92f;
            return std::pow((x + 0.055f) / 1.055f, 2.4f);
        };
        return { f(c.x), f(c.y), f(c.z) };
    }
    [[nodiscard]] inline UInt32 PackRGBA8(const Vector3& rgb, Scalar a) noexcept {
        auto clamp01 = [](Scalar v) noexcept -> Scalar { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); };
        UInt32 R = static_cast<UInt32>(clamp01(rgb.x) * 255.0f + 0.5f);
        UInt32 G = static_cast<UInt32>(clamp01(rgb.y) * 255.0f + 0.5f);
        UInt32 B = static_cast<UInt32>(clamp01(rgb.z) * 255.0f + 0.5f);
        UInt32 A = static_cast<UInt32>(clamp01(a) * 255.0f + 0.5f);
        return (A << 24) | (B << 16) | (G << 8) | R;
    }
    [[nodiscard]] inline void UnpackRGBA8(UInt32 p, Vector3& rgb, Scalar& a) noexcept {
        UInt32 R = (p) & 0xFFu;
        UInt32 G = (p >> 8) & 0xFFu;
        UInt32 B = (p >> 16) & 0xFFu;
        UInt32 A = (p >> 24) & 0xFFu;
        rgb = { R / 255.0f, G / 255.0f, B / 255.0f };
        a = A / 255.0f;
    }
    [[nodiscard]] inline UInt32 PackR11G11B10(const Vector3& rgb) noexcept {
        auto clampPos = [](Scalar v) noexcept -> Scalar { return v < 0.0f ? 0.0f : v; };
        Scalar rx = clampPos(rgb.x), gx = clampPos(rgb.y), bx = clampPos(rgb.z);
        HalfT rh = ScalarToHalf(rx);
        HalfT gh = ScalarToHalf(gx);
        HalfT bh = ScalarToHalf(bx);
        UInt16 rbits = static_cast<UInt16>(((rh.bits >> 10) & 0x1Fu) << 6 | ((rh.bits & 0x3FFu) >> 4));
        UInt16 gbits = static_cast<UInt16>(((gh.bits >> 10) & 0x1Fu) << 6 | ((gh.bits & 0x3FFu) >> 4));
        UInt16 bbits = static_cast<UInt16>(((bh.bits >> 10) & 0x1Fu) << 5 | ((bh.bits & 0x3FFu) >> 5));
        return static_cast<UInt32>(rbits) | (static_cast<UInt32>(gbits) << 11) | (static_cast<UInt32>(bbits) << 22);
    }
    [[nodiscard]] inline Vector3 UnpackR11G11B10(UInt32 p) noexcept {
        UInt32 rbits = (p) & 0x7FFu;
        UInt32 gbits = (p >> 11) & 0x7FFu;
        UInt32 bbits = (p >> 22) & 0x3FFu;
        UInt16 re = static_cast<UInt16>((rbits >> 6) & 0x1Fu);
        UInt16 rm = static_cast<UInt16>(rbits & 0x3Fu);
        UInt16 ge = static_cast<UInt16>((gbits >> 6) & 0x1Fu);
        UInt16 gm = static_cast<UInt16>(gbits & 0x3Fu);
        UInt16 be = static_cast<UInt16>((bbits >> 5) & 0x1Fu);
        UInt16 bm = static_cast<UInt16>(bbits & 0x1Fu);
        HalfT rh{ static_cast<UInt16>((re << 10) | (rm << 4)) };
        HalfT gh{ static_cast<UInt16>((ge << 10) | (gm << 4)) };
        HalfT bh{ static_cast<UInt16>((be << 10) | (bm << 5)) };
        return { HalfToScalar(rh), HalfToScalar(gh), HalfToScalar(bh) };
    }
}

