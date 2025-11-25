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

    std::atomic<unsigned long long> cntPushB{0}, cntPushF{0}, cntPopB{0}, cntPopF{0};
    std::atomic<bool> stop{false};
    auto startTime = steady_clock::now();
    auto endTime = startTime + seconds(30);

    std::thread reporter([&](){
        auto lb = cntPushB.load(std::memory_order_relaxed);
        auto lf = cntPushF.load(std::memory_order_relaxed);
        auto pb = cntPopB.load(std::memory_order_relaxed);
        auto pf = cntPopF.load(std::memory_order_relaxed);
        auto lastReport = startTime;
        while (!stop.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(seconds(1));
            auto now = steady_clock::now();
            if (now >= endTime) break;
            auto cb = cntPushB.load(std::memory_order_relaxed);
            auto cf = cntPushF.load(std::memory_order_relaxed);
            auto db = cntPopB.load(std::memory_order_relaxed);
            auto df = cntPopF.load(std::memory_order_relaxed);
            auto el = duration_cast<seconds>(now - lastReport).count();
            if (el > 0) {
                auto rb = (cb - lb) / static_cast<unsigned long long>(el);
                auto rf = (cf - lf) / static_cast<unsigned long long>(el);
                auto rpb = (db - pb) / static_cast<unsigned long long>(el);
                auto rpf = (df - pf) / static_cast<unsigned long long>(el);
                std::printf("[%lld秒] Deque: PushB=%llu/s, PushF=%llu/s, PopB=%llu/s, PopF=%llu/s\n", static_cast<long long>(duration_cast<seconds>(now - startTime).count()), rb, rf, rpb, rpf);
                lb = cb; lf = cf; pb = db; pf = df; lastReport = now;
            }
        }
    });

    std::vector<std::thread> workers; workers.reserve(Threads);
    for (int t = 0; t < Threads; ++t) {
        workers.emplace_back([&, t](){
            FastRng rng(static_cast<unsigned long long>(t + 1));
            Deque<UInt32> dq{};
            while (!stop.load(std::memory_order_relaxed)) {
                if (steady_clock::now() >= endTime) break;
                int op = static_cast<int>(rng.next() % 4u);
                if (op == 0) {
                    auto s = dq.PushBack(static_cast<UInt32>(rng.next()));
                    if (s.Ok()) { cntPushB.fetch_add(1, std::memory_order_relaxed); }
                } else if (op == 1) {
                    auto s = dq.PushFront(static_cast<UInt32>(rng.next()));
                    if (s.Ok()) { cntPushF.fetch_add(1, std::memory_order_relaxed); }
                } else if (op == 2) {
                    auto sz = dq.Size(); if (sz) { dq.PopBack(); cntPopB.fetch_add(1, std::memory_order_relaxed); }
                } else {
                    auto sz = dq.Size(); if (sz) { dq.PopFront(); cntPopF.fetch_add(1, std::memory_order_relaxed); }
                }
            }
        });
    }

    while (steady_clock::now() < endTime) { std::this_thread::sleep_for(milliseconds(100)); }
    stop.store(true, std::memory_order_relaxed);
    for (auto& th : workers) th.join();
    reporter.join();

    auto totalElapsed = duration_cast<seconds>(steady_clock::now() - startTime).count();
    std::printf("=== Deque 最终报告 (%lld秒) ===\n", static_cast<long long>(totalElapsed));
    std::printf("PushB=%llu, PushF=%llu, PopB=%llu, PopF=%llu\n", static_cast<unsigned long long>(cntPushB.load()), static_cast<unsigned long long>(cntPushF.load()), static_cast<unsigned long long>(cntPopB.load()), static_cast<unsigned long long>(cntPopF.load()));
    return 0;
}