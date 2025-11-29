import Lang;
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

    std::atomic<unsigned long long> cntPut{0}, cntGet{0}, cntErase{0};
    std::atomic<bool> stop{false};
    auto startTime = steady_clock::now();
    auto endTime = startTime + seconds(30);

    std::thread reporter([&](){
        auto lp = cntPut.load(std::memory_order_relaxed);
        auto lg = cntGet.load(std::memory_order_relaxed);
        auto le = cntErase.load(std::memory_order_relaxed);
        auto lastReport = startTime;
        while (!stop.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(seconds(1));
            auto now = steady_clock::now();
            if (now >= endTime) break;
            auto cp = cntPut.load(std::memory_order_relaxed);
            auto cg = cntGet.load(std::memory_order_relaxed);
            auto ce = cntErase.load(std::memory_order_relaxed);
            auto el = duration_cast<seconds>(now - lastReport).count();
            if (el > 0) {
                auto rp = (cp - lp) / static_cast<unsigned long long>(el);
                auto rg = (cg - lg) / static_cast<unsigned long long>(el);
                auto re = (ce - le) / static_cast<unsigned long long>(el);
                std::printf("[%lld秒] HashMap: Put=%llu/s, Get=%llu/s, Erase=%llu/s\n", static_cast<long long>(duration_cast<seconds>(now - startTime).count()), rp, rg, re);
                lp = cp; lg = cg; le = ce; lastReport = now;
            }
        }
    });

    std::vector<std::thread> workers; workers.reserve(Threads);
    for (int t = 0; t < Threads; ++t) {
        workers.emplace_back([&, t](){
            FastRng rng(static_cast<unsigned long long>(t + 1));
            HashMap<UInt32, UInt32> m{};
            while (!stop.load(std::memory_order_relaxed)) {
                if (steady_clock::now() >= endTime) break;
                int op = static_cast<int>(rng.next() % 3u);
                UInt32 key = static_cast<UInt32>(rng.next());
                if (op == 0) {
                    auto s = m.Put(key, static_cast<UInt32>(rng.next()));
                    if (s.Ok()) { cntPut.fetch_add(1, std::memory_order_relaxed); }
                } else if (op == 1) {
                    auto p = m.GetPtr(key);
                    if (p) { (void)*p; }
                    cntGet.fetch_add(1, std::memory_order_relaxed);
                } else {
                    bool b = m.Erase(key);
                    if (b) { cntErase.fetch_add(1, std::memory_order_relaxed); }
                }
            }
        });
    }

    while (steady_clock::now() < endTime) { std::this_thread::sleep_for(milliseconds(100)); }
    stop.store(true, std::memory_order_relaxed);
    for (auto& th : workers) th.join();
    reporter.join();

    auto totalElapsed = duration_cast<seconds>(steady_clock::now() - startTime).count();
    std::printf("=== HashMap 最终报告 (%lld秒) ===\n", static_cast<long long>(totalElapsed));
    std::printf("Put=%llu, Get=%llu, Erase=%llu\n", static_cast<unsigned long long>(cntPut.load()), static_cast<unsigned long long>(cntGet.load()), static_cast<unsigned long long>(cntErase.load()));
    return 0;
}