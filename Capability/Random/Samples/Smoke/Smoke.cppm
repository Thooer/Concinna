export module Random.Smoke;

import Lang;
import Cap.Random;

extern "C" int main() {
    using namespace Cap::Random;
    auto& rng = Default();
    Float32 acc = 0.0f;
    for (int i = 0; i < 1000; ++i) {
        Float32 u = Uniform01(rng);
        if (!(u >= 0.0f && u < 1.0f)) return 1;
        acc += u;
        auto x = UniformInt(rng, static_cast<UInt32>(0), static_cast<UInt32>(9));
        if (!(x <= 9u)) return 2;
        auto f = UniformFloat(rng, 5.0f, 10.0f);
        if (!(f >= 5.0f && f < 10.0f)) return 3;
    }
    return acc > 0.0f ? 0 : 4;
}
