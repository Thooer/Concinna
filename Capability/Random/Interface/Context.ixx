export module Cap.Random:Context;

import Lang;
import :Core;
import :Engines;

export namespace Cap::Random {
    [[nodiscard]] inline Pcg32& Default() noexcept {
        thread_local Pcg32 rng{};
        thread_local bool init = false;
        if (!init) {
            UInt64 seed = static_cast<UInt64>(reinterpret_cast<UIntPtr>(&rng));
            UInt64 seq = seed ^ 0x9E3779B97F4A7C15ull;
            rng.Seed(seed, seq);
            init = true;
        }
        return rng;
    }

    inline void SeedDefault(UInt64 seed, UInt64 seq) noexcept {
        Default().Seed(seed, seq);
    }
}
