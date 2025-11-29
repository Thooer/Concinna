import Lang;
import Containers;
import Memory;
import <chrono>;
import <cstdio>;
import <vector>;

extern "C" int main() {
    using namespace Containers;
    
    using namespace Memory;
    using namespace std::chrono;

    FrameAllocatorResource frame{ 1u << 30 };
    Allocator a{ &frame };
    String<Allocator> s{ a };

    const USize Step = static_cast<USize>(4096);
    const int Expansions = 8192;

    auto t0 = steady_clock::now();
    unsigned long long inPlace = 0, moved = 0;
    (void)s.Reserve(Step * 2);
    for (int e = 0; e < Expansions; ++e) {
        USize cap = s.Capacity();
        const void* p0 = static_cast<const void*>(s.Data());
        auto st = s.Reserve(cap + Step);
        if (st.Ok()) {
            const void* p1 = static_cast<const void*>(s.Data());
            if (p0 == p1) { ++inPlace; } else { ++moved; }
        }
    }
    auto t1 = steady_clock::now();
    double sec = static_cast<double>(duration_cast<microseconds>(t1 - t0).count()) / 1e6;
    std::printf("String(FrameAllocator) expansions=%d, inPlace=%llu, moved=%llu, ratio=%.2f%%, time=%.2fs\n", Expansions, inPlace, moved, (Expansions ? (100.0 * static_cast<double>(inPlace) / static_cast<double>(Expansions)) : 0.0), sec);
    return 0;
}