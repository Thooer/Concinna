import Language;
import Memory;
import Platform;
import <thread>;
import <vector>;
import <random>;
import <memory>;

extern "C" int main() {
    using namespace Memory;
    
    constexpr USize BlockSize = 64;
    constexpr USize Capacity = 1u << 18;
    PoolAllocatorResource pool{ BlockSize, Capacity };
    SmallObjectAllocatorResource small{ Capacity, &pool };
    EbrManager ebr{};

    auto reclaim_pool = [](void* p, void* ctx) noexcept {
        auto* res = static_cast<PoolAllocatorResource*>(ctx);
        res->Deallocate(MemoryBlock{ p, BlockSize }, Alignment::Default);
    };
    auto reclaim_small = [](void* p, void* ctx) noexcept {
        auto* res = static_cast<SmallObjectAllocatorResource*>(ctx);
        res->Deallocate(MemoryBlock{ p, 32 }, Alignment::Default);
    };
    constexpr int Threads = 32;
    constexpr int Ops = 20000;
    std::vector<std::thread> ts;
    ts.reserve(Threads);
    const int SharedSize = Threads * 64;
    std::unique_ptr<std::atomic<void*>[]> shared(new std::atomic<void*>[SharedSize]);
    for (int i = 0; i < SharedSize; ++i) { shared[i].store(nullptr, std::memory_order_relaxed); }
    std::atomic<int> idx{0};
    for (int t = 0; t < Threads; ++t) {
        ts.emplace_back([&](){
            std::mt19937_64 gen(static_cast<unsigned long long>(t + 1));
            std::uniform_int_distribution<int> dist(0, 3);
            for (int i = 0; i < Ops; ++i) {
                EbrManager::EpochGuard g{ ebr };
                int op = dist(gen);
                if (op == 0) {
                    auto r = pool.Allocate(BlockSize, Alignment::Default);
                    if (r.IsOk()) {
                        int j = idx.fetch_add(1) % SharedSize;
                        void* expected = nullptr;
                        if (!shared[j].compare_exchange_strong(expected, r.Value().ptr)) {
                            pool.Deallocate(MemoryBlock{ r.Value().ptr, BlockSize }, Alignment::Default);
                        }
                    }
                } else if (op == 1) {
                    auto r = small.Allocate(32, Alignment::Default);
                    if (r.IsOk()) {
                        int j = idx.fetch_add(1) % SharedSize;
                        void* expected = nullptr;
                        if (!shared[j].compare_exchange_strong(expected, r.Value().ptr)) {
                            small.Deallocate(MemoryBlock{ r.Value().ptr, 32 }, Alignment::Default);
                        }
                    }
                } else if (op == 2) {
                    int j = idx.fetch_add(1) % SharedSize;
                    void* p = shared[j].exchange(nullptr);
                    if (p) { ebr.Retire(p, &pool, reclaim_pool); }
                } else {
                    int j = idx.fetch_add(1) % SharedSize;
                    void* p = shared[j].exchange(nullptr);
                    if (p) { ebr.Retire(p, &small, reclaim_small); }
                }
                if ((i & 0xFF) == 0) { ebr.Advance(); ebr.Collect(); }
            }
        });
    }
    for (auto& th : ts) th.join();
    return 0;
}