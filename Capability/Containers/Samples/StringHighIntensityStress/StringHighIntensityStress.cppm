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

    std::atomic<unsigned long long> cntAppend{0}, cntReserve{0}, cntAssign{0}, cntClear{0};
    std::atomic<bool> stop{false};
    auto startTime = steady_clock::now();
    auto endTime = startTime + seconds(30);

    std::thread reporter([&](){
        auto la = cntAppend.load(std::memory_order_relaxed);
        auto lr = cntReserve.load(std::memory_order_relaxed);
        auto ls = cntAssign.load(std::memory_order_relaxed);
        auto lc = cntClear.load(std::memory_order_relaxed);
        auto lastReport = startTime;
        while (!stop.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(seconds(1));
            auto now = steady_clock::now();
            if (now >= endTime) break;
            auto ca = cntAppend.load(std::memory_order_relaxed);
            auto cr = cntReserve.load(std::memory_order_relaxed);
            auto cs = cntAssign.load(std::memory_order_relaxed);
            auto cc = cntClear.load(std::memory_order_relaxed);
            auto el = duration_cast<seconds>(now - lastReport).count();
            if (el > 0) {
                auto ra = (ca - la) / static_cast<unsigned long long>(el);
                auto rr = (cr - lr) / static_cast<unsigned long long>(el);
                auto rs = (cs - ls) / static_cast<unsigned long long>(el);
                auto rc = (cc - lc) / static_cast<unsigned long long>(el);
                std::printf("[%lld秒] String: Append=%llu/s, Reserve=%llu/s, Assign=%llu/s, Clear=%llu/s\n", static_cast<long long>(duration_cast<seconds>(now - startTime).count()), ra, rr, rs, rc);
                la = ca; lr = cr; ls = cs; lc = cc; lastReport = now;
            }
        }
    });

    std::vector<std::thread> workers; workers.reserve(Threads);
    for (int t = 0; t < Threads; ++t) {
        workers.emplace_back([&, t](){
            FastRng rng(static_cast<unsigned long long>(t + 1));
            String<> s{};
            Char8 buf[1024];
            for (int i = 0; i < 1024; ++i) { buf[i] = static_cast<Char8>('a' + (i % 26)); }
            while (!stop.load(std::memory_order_relaxed)) {
                if (steady_clock::now() >= endTime) break;
                int op = static_cast<int>(rng.next() % 4u);
                if (op == 0) {
                    USize n = static_cast<USize>((rng.next() % 128u) + 1u);
                    auto st = s.Append(buf, n);
                    if (st.Ok()) { cntAppend.fetch_add(1, std::memory_order_relaxed); }
                } else if (op == 1) {
                    USize cap = static_cast<USize>(s.Capacity() ? s.Capacity() * 2 : 64);
                    auto st = s.Reserve(cap);
                    if (st.Ok()) { cntReserve.fetch_add(1, std::memory_order_relaxed); }
                } else if (op == 2) {
                    USize n = static_cast<USize>((rng.next() % 64u) + 1u);
                    auto st = s.Assign(buf, n);
                    if (st.Ok()) { cntAssign.fetch_add(1, std::memory_order_relaxed); }
                } else {
                    s.Clear();
                    cntClear.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    while (steady_clock::now() < endTime) { std::this_thread::sleep_for(milliseconds(100)); }
    stop.store(true, std::memory_order_relaxed);
    for (auto& th : workers) th.join();
    reporter.join();

    auto totalElapsed = duration_cast<seconds>(steady_clock::now() - startTime).count();
    std::printf("=== String 最终报告 (%lld秒) ===\n", static_cast<long long>(totalElapsed));
    std::printf("Append=%llu, Reserve=%llu, Assign=%llu, Clear=%llu\n", static_cast<unsigned long long>(cntAppend.load()), static_cast<unsigned long long>(cntReserve.load()), static_cast<unsigned long long>(cntAssign.load()), static_cast<unsigned long long>(cntClear.load()));
    return 0;
}