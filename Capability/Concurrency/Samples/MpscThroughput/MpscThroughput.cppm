module;
#include <atomic>
#include <vector>
#include <thread>
#include <iostream>

export module Concurrency.MpscThroughput;
import Language;
import Platform;
import Concurrency;
namespace MpscDemo {
    using namespace std;
    using namespace std::chrono;

    void Producer(Concurrency::MPMCQueue<int>* q, int base, int count) noexcept {
        for (int i = 0; i < count; ++i) {
            (void)q->Enqueue(base + i);
        }
    }

    void Consumer(Concurrency::MPMCQueue<int>* q, std::atomic<long long>* consumed, long long target) noexcept {
        int v = 0;
        while (consumed->load(memory_order_relaxed) < target) {
            auto r = q->Dequeue(v);
            if (r.IsOk() && r.Value()) {
                consumed->fetch_add(1, memory_order_relaxed);
            } else {
                std::this_thread::yield();
            }
        }
    }
}

export int main() {
    using namespace MpscDemo;
    constexpr int producers = 32;
    constexpr int perProducer = 20000;
    const long long total = static_cast<long long>(producers) * perProducer;

    Concurrency::MPMCQueue<int> q{};
    std::atomic<long long> consumed{0};

    auto t0 = high_resolution_clock::now();

    std::thread cons(Consumer, &q, &consumed, total);
    std::vector<std::thread> ths;
    ths.reserve(producers);
    for (int i = 0; i < producers; ++i) {
        ths.emplace_back(Producer, &q, i * perProducer, perProducer);
    }
    for (auto& th : ths) th.join();
    cons.join();

    auto t1 = high_resolution_clock::now();
    auto ms = duration_cast<milliseconds>(t1 - t0).count();
    double mops = static_cast<double>(total) / (ms / 1000.0) / 1e6;

    std::cout << "MPSC throughput: " << total << " ops in " << ms << " ms (" << mops << " Mops/s)\n";
    return 0;
}