export module Cap.Random:Distribution;

import Lang;
import :Core;
import <cmath>;

export namespace Cap::Random {
    template<RngEngine E>
    [[nodiscard]] inline Float32 Uniform01(E& e) noexcept {
        constexpr Float32 k = 1.0f / 4294967296.0f;
        return static_cast<Float32>(e.Next()) * k;
    }

    template<RngEngine E>
    [[nodiscard]] inline Float32 UniformFloat(E& e, Float32 min, Float32 max) noexcept {
        Float32 u = Uniform01(e);
        return min + (max - min) * u;
    }

    template<RngEngine E, UnsignedIntegral T>
    [[nodiscard]] inline T UniformInt(E& e, T min, T max) noexcept {
        if (max <= min) return min;
        const UInt32 bound = static_cast<UInt32>(static_cast<UInt64>(max - min) + 1ull);
        const UInt32 threshold = static_cast<UInt32>(-bound) % bound;
        UInt32 r;
        do { r = e.Next(); } while (r < threshold);
        return static_cast<T>(min + (r % bound));
    }

    template<RngEngine E>
    [[nodiscard]] inline Float32 Normal(E& e, Float32 mean, Float32 stddev) noexcept {
        Float32 u1 = Uniform01(e);
        Float32 u2 = Uniform01(e);
        if (u1 <= 0.0f) { u1 = 1e-7f; }
        constexpr Float32 kTwoPi = 6.28318530717958647692f;
        Float32 r = std::sqrt(-2.0f * std::log(u1));
        Float32 theta = kTwoPi * u2;
        Float32 z = r * std::cos(theta);
        return mean + z * stddev;
    }
}
