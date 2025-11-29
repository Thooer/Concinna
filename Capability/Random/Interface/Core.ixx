export module Cap.Random:Core;

import Lang;

export namespace Cap::Random {
    template<typename T>
    concept RngEngine = requires(T& t) {
        { t.Next() } -> UnsignedIntegral;
        { T::Min() } -> UnsignedIntegral;
        { T::Max() } -> UnsignedIntegral;
    };
}
