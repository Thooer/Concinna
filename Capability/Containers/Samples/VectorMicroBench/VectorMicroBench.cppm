import Language;
import Containers;
import <chrono>;
import <cstdio>;
import <vector>;
import <algorithm>;
import <cmath>;

struct StatsResult {
    double mean;
    double stddev;
    double min;
    double max;
    double cv;
};

extern "C" int main() {
    using namespace Containers;
    
    using namespace std::chrono;

    const USize NReserved = static_cast<USize>(10'000'000);
    const USize NGrowth = static_cast<USize>(8'000'000);
    const int Trials = 30;

    // 统计分析辅助函数
    auto calculate_stats = [](const std::vector<double>& values) -> StatsResult {
        double sum = 0.0, sum_sq = 0.0;
        double min_val = values[0], max_val = values[0];
        for (double v : values) {
            sum += v;
            sum_sq += v * v;
            min_val = std::min(min_val, v);
            max_val = std::max(max_val, v);
        }
        double mean = sum / values.size();
        double variance = (sum_sq / values.size()) - (mean * mean);
        double stddev = std::sqrt(std::max(0.0, variance));
        double cv = mean > 0 ? (stddev / mean) * 100.0 : 0.0; // 变异系数
        
        return StatsResult{mean, stddev, min_val, max_val, cv};
    };

    std::printf("=== Vector容器微基准测试 ===\n");
    std::printf("试验次数: %d\n\n", Trials);

    // 测试1: PushUnsafe (预分配)
    {
        std::printf("测试1: PushUnsafe(预分配 %u 元素)\n", NReserved);
        std::vector<double> throughputs, nsops;
        
        for (int k = 0; k < Trials; ++k) {
            Vector<UInt32> v{};
            (void)v.Reserve(NReserved);
            auto t0 = steady_clock::now();
            for (USize i = 0; i < NReserved; ++i) { 
                v.EmplaceBackUnsafe(static_cast<UInt32>(i)); 
            }
            auto t1 = steady_clock::now();
            double ops = static_cast<double>(NReserved);
            double sec = static_cast<double>(duration_cast<microseconds>(t1 - t0).count()) / 1e6;
            double th = ops / sec;
            double nsop = static_cast<double>(duration_cast<nanoseconds>(t1 - t0).count()) / ops;
            
            throughputs.push_back(th);
            nsops.push_back(nsop);
            
            std::printf("  试验 %2d: %.2f M ops/s, %.2f ns/op\n", k + 1, th / 1e6, nsop);
        }
        
        auto th_stats = calculate_stats(throughputs);
        auto ns_stats = calculate_stats(nsops);
        
        std::printf("  === 吞吐量统计 ===\n");
        std::printf("    平均:   %.2f M ops/s\n", th_stats.mean / 1e6);
        std::printf("    标准差: %.2f M ops/s (%.1f%%)\n", th_stats.stddev / 1e6, th_stats.cv);
        std::printf("    最小:   %.2f M ops/s\n", th_stats.min / 1e6);
        std::printf("    最大:   %.2f M ops/s\n", th_stats.max / 1e6);
        std::printf("  === 时延统计 ===\n");
        std::printf("    平均:   %.2f ns/op\n", ns_stats.mean);
        std::printf("    标准差: %.2f ns/op (%.1f%%)\n", ns_stats.stddev, ns_stats.cv);
        std::printf("    最小:   %.2f ns/op\n", ns_stats.min);
        std::printf("    最大:   %.2f ns/op\n", ns_stats.max);
        std::printf("\n");
    }

    // 测试2: Push (动态增长)
    {
        std::printf("测试2: Push(从0增长 %u 元素)\n", NGrowth);
        std::vector<double> throughputs, nsops, grows;
        
        for (int k = 0; k < Trials; ++k) {
            Vector<UInt32> v{};
            auto t0 = steady_clock::now();
            USize lastCap = 0;
            USize grow_count = 0;
            for (USize i = 0; i < NGrowth; ++i) {
                auto s = v.EmplaceBack(static_cast<UInt32>(i));
                if (!s.Ok()) { break; }
                USize cap = v.Capacity();
                if (cap != lastCap) { ++grow_count; lastCap = cap; }
            }
            auto t1 = steady_clock::now();
            double ops = static_cast<double>(v.Size());
            double sec = static_cast<double>(duration_cast<microseconds>(t1 - t0).count()) / 1e6;
            double th = ops / sec;
            double nsop = static_cast<double>(duration_cast<nanoseconds>(t1 - t0).count()) / ops;
            
            throughputs.push_back(th);
            nsops.push_back(nsop);
            grows.push_back(static_cast<double>(grow_count));
            
            std::printf("  试验 %2d: %.2f M ops/s, %.2f ns/op, 增长%u次\n", k + 1, th / 1e6, nsop, grow_count);
        }
        
        auto th_stats = calculate_stats(throughputs);
        auto ns_stats = calculate_stats(nsops);
        auto grow_stats = calculate_stats(grows);
        
        std::printf("  === 吞吐量统计 ===\n");
        std::printf("    平均:   %.2f M ops/s\n", th_stats.mean / 1e6);
        std::printf("    标准差: %.2f M ops/s (%.1f%%)\n", th_stats.stddev / 1e6, th_stats.cv);
        std::printf("    最小:   %.2f M ops/s\n", th_stats.min / 1e6);
        std::printf("    最大:   %.2f M ops/s\n", th_stats.max / 1e6);
        std::printf("  === 时延统计 ===\n");
        std::printf("    平均:   %.2f ns/op\n", ns_stats.mean);
        std::printf("    标准差: %.2f ns/op (%.1f%%)\n", ns_stats.stddev, ns_stats.cv);
        std::printf("    最小:   %.2f ns/op\n", ns_stats.min);
        std::printf("    最大:   %.2f ns/op\n", ns_stats.max);
        std::printf("  === 增长次数统计 ===\n");
        std::printf("    平均:   %.1f 次\n", grow_stats.mean);
        std::printf("    标准差: %.1f 次 (%.1f%%)\n", grow_stats.stddev, grow_stats.cv);
        std::printf("    最小:   %.0f 次\n", grow_stats.min);
        std::printf("    最大:   %.0f 次\n", grow_stats.max);
        std::printf("\n");
    }

    // 测试3: EraseUnordered(中部删除)
    {
        const USize M = static_cast<USize>(8'000'000);
        std::printf("测试3: EraseUnordered(中部删除 %u 元素)\n", M);
        std::vector<double> throughputs, nsops;
        
        for (int k = 0; k < Trials; ++k) {
            Vector<UInt32> v{};
            (void)v.Reserve(M);
            for (USize i = 0; i < M; ++i) { v.EmplaceBackUnsafe(static_cast<UInt32>(i)); }
            
            auto t0 = steady_clock::now();
            for (USize i = 0; i < M; ++i) {
                USize sz = v.Size();
                USize mid = sz ? (sz >> 1) : 0;
                if (sz) v.EraseUnordered(mid);
            }
            auto t1 = steady_clock::now();
            double ops = static_cast<double>(M);
            double sec = static_cast<double>(duration_cast<microseconds>(t1 - t0).count()) / 1e6;
            double th = ops / sec;
            double nsop = static_cast<double>(duration_cast<nanoseconds>(t1 - t0).count()) / ops;
            
            throughputs.push_back(th);
            nsops.push_back(nsop);
            
            std::printf("  试验 %2d: %.2f M ops/s, %.2f ns/op\n", k + 1, th / 1e6, nsop);
        }
        
        auto th_stats = calculate_stats(throughputs);
        auto ns_stats = calculate_stats(nsops);
        
        std::printf("  === 吞吐量统计 ===\n");
        std::printf("    平均:   %.2f M ops/s\n", th_stats.mean / 1e6);
        std::printf("    标准差: %.2f M ops/s (%.1f%%)\n", th_stats.stddev / 1e6, th_stats.cv);
        std::printf("    最小:   %.2f M ops/s\n", th_stats.min / 1e6);
        std::printf("    最大:   %.2f M ops/s\n", th_stats.max / 1e6);
        std::printf("  === 时延统计 ===\n");
        std::printf("    平均:   %.2f ns/op\n", ns_stats.mean);
        std::printf("    标准差: %.2f ns/op (%.1f%%)\n", ns_stats.stddev, ns_stats.cv);
        std::printf("    最小:   %.2f ns/op\n", ns_stats.min);
        std::printf("    最大:   %.2f ns/op\n", ns_stats.max);
        std::printf("\n");
    }

    // 总结对比
    std::printf("=== 测试总结 ===\n");
    std::printf("测试环境: Windows x64, 预分配模式 vs 动态增长 vs 无序删除\n");
    std::printf("性能排序 (按吞吐量): 预分配 > 动态增长 > 无序删除\n");
    std::printf("稳定性评估: 变异系数 < 5%% 为稳定，5-10%% 为一般，>10%% 为不稳定\n");

    return 0;
}