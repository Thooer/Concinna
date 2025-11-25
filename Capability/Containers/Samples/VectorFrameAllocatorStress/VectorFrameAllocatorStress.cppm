import Language;
import Containers;
import Memory;
import <chrono>;
import <cstdio>;

extern "C" int main() {
    using namespace Containers;
    
    using namespace Memory;
    using namespace std::chrono;

    FrameAllocatorResource frame{ 1u << 30 }; // 1 GiB
    Allocator a{ &frame };
    Vector<UInt32, Allocator> v{ a };

    const USize Step = static_cast<USize>(4096);
    const int Expansions = 8192;

    (void)v.Reserve(Step);
    USize cap = v.Capacity();
    unsigned long long inPlace = 0, moved = 0;
    auto t0 = steady_clock::now();
    for (int e = 0; e < Expansions; ++e) {
        void* p0 = static_cast<void*>(v.Data());
        auto s = v.Reserve(cap + Step);
        if (s.Ok()) {
            void* p1 = static_cast<void*>(v.Data());
            if (p0 == p1) { ++inPlace; } else { ++moved; }
            cap = v.Capacity();
        }
    }
    auto t1 = steady_clock::now();
    double sec = static_cast<double>(duration_cast<microseconds>(t1 - t0).count()) / 1e6;
    std::printf("Vector(FrameAllocator) expansions=%d, inPlace=%llu, moved=%llu, ratio=%.2f%%, time=%.2fs\n", Expansions, inPlace, moved, (Expansions ? (100.0 * static_cast<double>(inPlace) / static_cast<double>(Expansions)) : 0.0), sec);
    return 0;
}