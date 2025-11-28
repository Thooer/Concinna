export module Cap.Random:Engines;

import Language;
import :Core;

export namespace Cap::Random {
    struct Pcg32 {
        UInt64 state{ 0x853c49e6748fea9bull };
        UInt64 inc{ 0xda3e39cb94b95bdbull };

        static constexpr UInt32 Min() noexcept { return 0u; }
        static constexpr UInt32 Max() noexcept { return 0xFFFFFFFFu; }

        void Seed(UInt64 seed, UInt64 seq) noexcept {
            state = 0u;
            inc = (seq << 1u) | 1u;
            (void)Next();
            state += seed;
            (void)Next();
        }

        [[nodiscard]] UInt32 Next() noexcept {
            UInt64 old = state;
            state = old * 6364136223846793005ull + inc;
            UInt32 xorshifted = static_cast<UInt32>(((old >> 18u) ^ old) >> 27u);
            UInt32 rot = static_cast<UInt32>(old >> 59u);
            return (xorshifted >> rot) | (xorshifted << ((-static_cast<Int32>(rot)) & 31));
        }
    };
    static_assert(RngEngine<Pcg32>);
}
