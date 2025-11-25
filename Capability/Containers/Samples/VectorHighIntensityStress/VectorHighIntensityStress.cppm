import Language;
import Containers;
import <atomic>;
import <thread>;
import <vector>;
import <chrono>;
import <cstdio>;

struct FastRng { unsigned long long state; FastRng(unsigned long long seed) : state(seed | 1ull) {} unsigned int next() { unsigned long long x = state; x ^= x >> 12; x ^= x << 25; x ^= x >> 27; state = x; return static_cast<unsigned int>(((x * 0x2545F4914F6CDD1Dull) >> 32)); } };

extern "C" int main() {
    using namespace Containers;
    
    using namespace std::chrono;

    const unsigned int hw = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 8u;
    const int Threads = static_cast<int>(hw * 2u);

    std::atomic<unsigned long long> cntPush{0}, cntErase{0}, cntReserve{0};
    std::atomic<bool> stop{false};
    auto startTime = steady_clock::now();
    auto endTime = startTime + seconds(30);

    std::thread reporter([&](){
        auto lp = cntPush.load(std::memory_order_relaxed);
        auto le = cntErase.load(std::memory_order_relaxed);
        auto lr = cntReserve.load(std::memory_order_relaxed);
        auto lastReport = startTime;
        while (!stop.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(seconds(1));
            auto now = steady_clock::now();
            if (now >= endTime) break;
            auto cp = cntPush.load(std::memory_order_relaxed);
            auto ce = cntErase.load(std::memory_order_relaxed);
            auto cr = cntReserve.load(std::memory_order_relaxed);
            auto el = duration_cast<seconds>(now - lastReport).count();
            if (el > 0) {
                auto rp = (cp - lp) / static_cast<unsigned long long>(el);
                auto re = (ce - le) / static_cast<unsigned long long>(el);
                auto rr = (cr - lr) / static_cast<unsigned long long>(el);
                std::printf("[%lld秒] Vector: Push=%llu/s, Erase=%llu/s, Reserve=%llu/s\n", static_cast<long long>(duration_cast<seconds>(now - startTime).count()), rp, re, rr);
                lp = cp; le = ce; lr = cr; lastReport = now;
            }
        }
    });

    std::vector<std::thread> workers; workers.reserve(Threads);
    for (int t = 0; t < Threads; ++t) {
        workers.emplace_back([&, t](){
            FastRng rng(static_cast<unsigned long long>(t + 1));
            Vector<UInt32> v{};
            (void)v.Reserve(1u << 17);
            unsigned long long lp = 0, le = 0, lr = 0;
            unsigned int ops = 0;
            const unsigned int CommitMask = 4095u;
            while (!stop.load(std::memory_order_relaxed)) {
                unsigned int r = rng.next() & 255u;
                if (r < 128u) {
                    v.EmplaceBackUnsafe(static_cast<UInt32>(rng.next()));
                    ++lp;
                } else if (r < 255u) {
                    auto sz = v.Size();
                    if (sz > 0) { v.EraseUnordered(static_cast<USize>(rng.next() % static_cast<unsigned int>(sz))); ++le; }
                } else {
                    auto cap = v.Capacity();
                    USize target = cap ? cap * 2 : 64;
                    auto s = v.Reserve(target);
                    if (s.Ok()) { ++lr; }
                }
                ++ops;
                if ((ops & CommitMask) == 0u) {
                    if (lp) { cntPush.fetch_add(lp, std::memory_order_relaxed); lp = 0; }
                    if (le) { cntErase.fetch_add(le, std::memory_order_relaxed); le = 0; }
                    if (lr) { cntReserve.fetch_add(lr, std::memory_order_relaxed); lr = 0; }
                    if (steady_clock::now() >= endTime) break;
                }
            }
            if (lp) { cntPush.fetch_add(lp, std::memory_order_relaxed); }
            if (le) { cntErase.fetch_add(le, std::memory_order_relaxed); }
            if (lr) { cntReserve.fetch_add(lr, std::memory_order_relaxed); }
        });
    }

    while (steady_clock::now() < endTime) { std::this_thread::sleep_for(milliseconds(100)); }
    stop.store(true, std::memory_order_relaxed);
    for (auto& th : workers) th.join();
    reporter.join();

    auto totalElapsed = duration_cast<seconds>(steady_clock::now() - startTime).count();
    std::printf("=== Vector 最终报告 (%lld秒) ===\n", static_cast<long long>(totalElapsed));
    std::printf("Push=%llu, Erase=%llu, Reserve=%llu\n", static_cast<unsigned long long>(cntPush.load()), static_cast<unsigned long long>(cntErase.load()), static_cast<unsigned long long>(cntReserve.load()));
    return 0;
}