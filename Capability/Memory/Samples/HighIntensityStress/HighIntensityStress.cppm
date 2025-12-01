import Lang;
import Memory;
import Prm.Threading;
import <atomic>;
import <thread>;
import <vector>;
import <chrono>;
import <memory>;
import <cstdio>;

struct FastRng { unsigned long long state; FastRng(unsigned long long seed) : state(seed | 1ull) {} unsigned int next() { unsigned long long x = state; x ^= x >> 12; x ^= x << 25; x ^= x >> 27; state = x; return static_cast<unsigned int>(((x * 0x2545F4914F6CDD1Dull) >> 32)); } };

extern "C" int main() {
    using namespace Memory;
    
    using namespace std::chrono;

    constexpr USize BlockSize = 64;
    constexpr USize Capacity = 1u << 20;
    PoolAllocatorResource pool{ BlockSize, Capacity };
    SmallObjectAllocatorResource small{ Capacity, &pool };
    EbrManager ebr{};

    std::atomic<unsigned int> gate{0};

    const unsigned int hw = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 8u;
    const int Threads = static_cast<int>(hw * 2u);
    const int SlotsPerThread = 256;
    const int Slots = Threads * SlotsPerThread;
    
    // 解决伪共享问题：每个槽位用64字节对齐
    struct alignas(64) AlignedSlot {
        std::atomic<void*> ptr{nullptr};
        char pad[64 - sizeof(std::atomic<void*>)]; // 填充到64字节
    };
    std::unique_ptr<AlignedSlot[]> slots(new AlignedSlot[Slots]);
    for (int i = 0; i < Slots; ++i) { slots[i].ptr.store(nullptr, std::memory_order_relaxed); }

    std::atomic<unsigned long long> cntAllocPool{0}, cntAllocSmall{0}, cntRetirePool{0}, cntRetireSmall{0};
    std::atomic<bool> stop{false};

    auto startTime = steady_clock::now();
    auto endTime = startTime + seconds(30);

    std::thread collector([&](){
        while (!stop.load(std::memory_order_relaxed)) {
            ebr.Advance();
            ebr.Collect();
            gate.fetch_add(1, std::memory_order_relaxed);
            Prm::WakeByAddressAll(&gate);
            std::this_thread::sleep_for(milliseconds(2));
            if (steady_clock::now() >= endTime) break;
        }
    });

    // 每秒报告线程 - 修正速率计算
    std::thread reporter([&](){
        auto lastAllocPool = cntAllocPool.load(std::memory_order_relaxed);
        auto lastAllocSmall = cntAllocSmall.load(std::memory_order_relaxed);
        auto lastRetirePool = cntRetirePool.load(std::memory_order_relaxed);
        auto lastRetireSmall = cntRetireSmall.load(std::memory_order_relaxed);
        auto lastReportTime = startTime;
        
        while (!stop.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(seconds(1));
            auto now = steady_clock::now();
            if (now >= endTime) break;
            
            auto currentAllocPool = cntAllocPool.load(std::memory_order_relaxed);
            auto currentAllocSmall = cntAllocSmall.load(std::memory_order_relaxed);
            auto currentRetirePool = cntRetirePool.load(std::memory_order_relaxed);
            auto currentRetireSmall = cntRetireSmall.load(std::memory_order_relaxed);
            
            auto elapsed = duration_cast<seconds>(now - lastReportTime).count();
            if (elapsed > 0) {
                auto allocPoolRate = (currentAllocPool - lastAllocPool) / elapsed;
                auto allocSmallRate = (currentAllocSmall - lastAllocSmall) / elapsed;
                auto retirePoolRate = (currentRetirePool - lastRetirePool) / elapsed;
                auto retireSmallRate = (currentRetireSmall - lastRetireSmall) / elapsed;
                
                std::printf("[%02ld秒] 分配: Pool=%llu/s, Small=%llu/s | 回收: Pool=%llu/s, Small=%llu/s | 总计: %lld\n",
                    duration_cast<seconds>(now - startTime).count(),
                    allocPoolRate, allocSmallRate,
                    retirePoolRate, retireSmallRate,
                    currentAllocPool + currentAllocSmall
                );
                
                lastAllocPool = currentAllocPool;
                lastAllocSmall = currentAllocSmall;
                lastRetirePool = currentRetirePool;
                lastRetireSmall = currentRetireSmall;
                lastReportTime = now;
            }
        }
    });

    std::vector<std::thread> workers;
    workers.reserve(Threads);
    for (int t = 0; t < Threads; ++t) {
        workers.emplace_back([&, t](){
            FastRng rng(static_cast<unsigned long long>(t + 1));
            while (!stop.load(std::memory_order_relaxed)) {
                if (steady_clock::now() >= endTime) break;
                EbrManager::EpochGuard g{ ebr };
                int op = static_cast<int>(rng.next() % 4u);
                const int base = t * SlotsPerThread;
                const bool cross = (rng.next() & 31u) == 0u;
                if (op == 0) {
                    auto r = pool.Allocate(BlockSize, Alignment::Default);
                    if (r.IsOk()) {
                        int j = r.Value().ptr ? (cross ? static_cast<int>(rng.next() % static_cast<unsigned int>(Slots)) : base + static_cast<int>(rng.next() % static_cast<unsigned int>(SlotsPerThread))) : 0;
                        void* expected = nullptr;
                        if (!slots[j].ptr.compare_exchange_strong(expected, r.Value().ptr)) {
                            pool.Deallocate(MemoryBlock{ r.Value().ptr, BlockSize }, Alignment::Default);
                        } else {
                            cntAllocPool.fetch_add(1, std::memory_order_relaxed);
                        }
                    }
                } else if (op == 1) {
                    auto r = small.Allocate(32, Alignment::Default);
                    if (r.IsOk()) {
                        int j = r.Value().ptr ? (cross ? static_cast<int>(rng.next() % static_cast<unsigned int>(Slots)) : base + static_cast<int>(rng.next() % static_cast<unsigned int>(SlotsPerThread))) : 0;
                        void* expected = nullptr;
                        if (!slots[j].ptr.compare_exchange_strong(expected, r.Value().ptr)) {
                            small.Deallocate(MemoryBlock{ r.Value().ptr, 32 }, Alignment::Default);
                        } else {
                            cntAllocSmall.fetch_add(1, std::memory_order_relaxed);
                        }
                    }
                } else if (op == 2) {
                    int j = cross ? static_cast<int>(rng.next() % static_cast<unsigned int>(Slots)) : base + static_cast<int>(rng.next() % static_cast<unsigned int>(SlotsPerThread));
                    void* p = slots[j].ptr.exchange(nullptr);
                    if (p) { 
                        ebr.Retire(p, &pool, [](void* x, void* ctx) noexcept { 
                            auto* res = static_cast<PoolAllocatorResource*>(ctx); 
                            res->Deallocate(MemoryBlock{ x, BlockSize }, Alignment::Default); 
                        }); 
                        cntRetirePool.fetch_add(1, std::memory_order_relaxed); 
                    }
                } else if (op == 3) {
                    int j = cross ? static_cast<int>(rng.next() % static_cast<unsigned int>(Slots)) : base + static_cast<int>(rng.next() % static_cast<unsigned int>(SlotsPerThread));
                    void* p = slots[j].ptr.exchange(nullptr);
                    if (p) { 
                        ebr.Retire(p, &small, [](void* x, void* ctx) noexcept { 
                            auto* res = static_cast<SmallObjectAllocatorResource*>(ctx); 
                            res->Deallocate(MemoryBlock{ x, 32 }, Alignment::Default); 
                        }); 
                        cntRetireSmall.fetch_add(1, std::memory_order_relaxed); 
                    }
                }
            }
        });
    }

    while (steady_clock::now() < endTime) { std::this_thread::sleep_for(milliseconds(200)); }
    stop.store(true, std::memory_order_relaxed);
    Prm::WakeByAddressAll(&gate);
    for (auto& th : workers) th.join();
    collector.join();
    reporter.join();

    auto totalElapsed = duration_cast<seconds>(steady_clock::now() - startTime).count();
    std::printf("\n=== 最终测试报告 (测试时长: %ld秒) ===\n", totalElapsed);
    std::printf("总分配次数: Pool=%llu, Small=%llu, 总计=%llu\n", 
        static_cast<unsigned long long>(cntAllocPool.load(std::memory_order_relaxed)),
        static_cast<unsigned long long>(cntAllocSmall.load(std::memory_order_relaxed)),
        static_cast<unsigned long long>(cntAllocPool.load(std::memory_order_relaxed) + cntAllocSmall.load(std::memory_order_relaxed)));
    std::printf("总回收次数: Pool=%llu, Small=%llu, 总计=%llu\n", 
        static_cast<unsigned long long>(cntRetirePool.load(std::memory_order_relaxed)),
        static_cast<unsigned long long>(cntRetireSmall.load(std::memory_order_relaxed)),
        static_cast<unsigned long long>(cntRetirePool.load(std::memory_order_relaxed) + cntRetireSmall.load(std::memory_order_relaxed)));
    std::printf("平均每秒分配: Pool=%llu/s, Small=%llu/s, 总计=%llu/s\n", 
        cntAllocPool.load(std::memory_order_relaxed) / totalElapsed,
        cntAllocSmall.load(std::memory_order_relaxed) / totalElapsed,
        (cntAllocPool.load(std::memory_order_relaxed) + cntAllocSmall.load(std::memory_order_relaxed)) / totalElapsed);
    std::printf("平均每秒回收: Pool=%llu/s, Small=%llu/s, 总计=%llu/s\n", 
        cntRetirePool.load(std::memory_order_relaxed) / totalElapsed,
        cntRetireSmall.load(std::memory_order_relaxed) / totalElapsed,
        (cntRetirePool.load(std::memory_order_relaxed) + cntRetireSmall.load(std::memory_order_relaxed)) / totalElapsed);
    
    // 计算内存效率
    auto totalAlloc = cntAllocPool.load(std::memory_order_relaxed) + cntAllocSmall.load(std::memory_order_relaxed);
    auto totalRetire = cntRetirePool.load(std::memory_order_relaxed) + cntRetireSmall.load(std::memory_order_relaxed);
    std::printf("内存管理效率: %.2f%% (回收/分配)\n", 
        totalAlloc > 0 ? (static_cast<double>(totalRetire) / totalAlloc * 100.0) : 0.0);
    
    return 0;
}
