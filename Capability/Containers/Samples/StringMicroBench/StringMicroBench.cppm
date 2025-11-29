import Lang;
 import Containers;
 import <vector>;
 import <algorithm>;
 import <cmath>;
 import <chrono>;
 import <random>;
 import <string>;
 import <cstdio>;
 import <limits>;
 import <cstdlib>;
 import <cstring>;
 import <new>;

 struct StatsResult {
     double mean{0.0};
     double stddev{0.0};
     double min_value{0.0};
     double max_value{0.0};
     double cv{0.0};  // 变异系数
 };

 [[nodiscard]] StatsResult calculate_stats(const std::vector<double>& values) noexcept {
     if (values.empty()) return {};
     
     double sum = 0.0;
     double min_val = std::numeric_limits<double>::max();
     double max_val = std::numeric_limits<double>::lowest();
     
     for (double v : values) {
         sum += v;
         min_val = std::min(min_val, v);
         max_val = std::max(max_val, v);
     }
     
     double mean = sum / static_cast<double>(values.size());
     double variance_sum = 0.0;
     
     for (double v : values) {
         double diff = v - mean;
         variance_sum += diff * diff;
     }
     
     double stddev = std::sqrt(variance_sum / static_cast<double>(values.size()));
     double cv = (mean > 0.0) ? (stddev / mean) * 100.0 : 0.0;
     
     return {mean, stddev, min_val, max_val, cv};
 }

 [[nodiscard]] const char* format_throughput(double ops_per_sec, char* buffer, size_t buffer_size) noexcept {
     if (ops_per_sec >= 1000.0 * 1000.0 * 1000.0) {
         snprintf(buffer, buffer_size, "%.2f G ops/s", ops_per_sec / (1000.0 * 1000.0 * 1000.0));
     } else if (ops_per_sec >= 1000.0 * 1000.0) {
         snprintf(buffer, buffer_size, "%.2f M ops/s", ops_per_sec / (1000.0 * 1000.0));
     } else if (ops_per_sec >= 1000.0) {
         snprintf(buffer, buffer_size, "%.2f K ops/s", ops_per_sec / 1000.0);
     } else {
         snprintf(buffer, buffer_size, "%.2f ops/s", ops_per_sec);
     }
     return buffer;
 }

 [[nodiscard]] const char* format_latency(double ns_per_op, char* buffer, size_t buffer_size) noexcept {
     if (ns_per_op >= 1000.0 * 1000.0) {
         snprintf(buffer, buffer_size, "%.2f ms/op", ns_per_op / (1000.0 * 1000.0));
     } else if (ns_per_op >= 1000.0) {
         snprintf(buffer, buffer_size, "%.2f us/op", ns_per_op / 1000.0);
     } else {
         snprintf(buffer, buffer_size, "%.2f ns/op", ns_per_op);
     }
     return buffer;
 }

 extern "C" int main() {
     using namespace Containers;
     
     using namespace std::chrono;
     
     std::random_device rd;
     std::mt19937 rng(rd());
     
     const int trials = 30;
     const int iterations = 1000000; // 100万次操作
     
     // 测试字符串（模拟实际使用场景）
     const Char8 test_data[] = {
         'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', ' ', 'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 
         'a', ' ', 't', 'e', 's', 't', ' ', 's', 't', 'r', 'i', 'n', 'g', ' ', 'f', 'o', 'r', ' ', 'b', 'e', 'n', 
         'c', 'h', 'm', 'a', 'r', 'k', 'i', 'n', 'g', ' ', 'S', 't', 'r', 'i', 'n', 'g', ' ', 'o', 'p', 'e', 'r', 
         'a', 't', 'i', 'o', 'n', 's', '.', ' ', 'W', 'e', ' ', 'n', 'e', 'e', 'd', ' ', 't', 'o', ' ', 't', 'e', 
         's', 't', ' ', 'b', 'o', 't', 'h', ' ', 's', 'm', 'a', 'l', 'l', ' ', 's', 't', 'r', 'i', 'n', 'g', ' ', 
         'o', 'p', 't', 'i', 'm', 'i', 'z', 'a', 't', 'i', 'o', 'n', ' ', 'a', 'n', 'd', ' ', 'h', 'e', 'a', 'p', 
         ' ', 'a', 'l', 'l', 'o', 'c', 'a', 't', 'i', 'o', 'n', ' ', 's', 'c', 'e', 'n', 'a', 'r', 'i', 'o', 's', '.', '\0'
     };
     const USize test_data_len = static_cast<USize>(sizeof(test_data) / sizeof(test_data[0]) - 1);
     const USize sso_threshold = String<>::kSSOCap; // 24 bytes
     
     // 缓冲区定义
     char buf1[64], buf2[64];
     
     // 测试场景1: SSO范围内追加操作 (Append within SSO)
     std::printf("=== String Micro Benchmarks ===\n\n");
     std::printf("测试场景1: SSO范围内追加操作 (长度 < %zu)\n", sso_threshold);
     
     std::vector<double> throughputs1;
     std::vector<double> latencies1;
     
     for (int trial = 1; trial <= trials; ++trial) {
         auto start = high_resolution_clock::now();
         String<> s{};
         
         for (int i = 0; i < iterations; ++i) {
             s.Clear();
             Char8 small_buf[16];
             for (int j = 0; j < 15; ++j) {
                 small_buf[j] = static_cast<Char8>('a' + (j % 26));
             }
             small_buf[15] = '\0';
             auto st = s.Append(small_buf, static_cast<USize>(15));
         }
         
         auto end = high_resolution_clock::now();
         auto duration = duration_cast<nanoseconds>(end - start).count();
         
         double throughput = (static_cast<double>(iterations) / duration) * 1e9;
         double latency = duration / static_cast<double>(iterations);
         
         throughputs1.push_back(throughput);
         latencies1.push_back(latency);
         
         std::printf("试验 %2d: %s, %s\n", trial, 
                    format_throughput(throughput, buf1, sizeof(buf1)),
                    format_latency(latency, buf2, sizeof(buf2)));
     }
     
     auto stats1 = calculate_stats(throughputs1);
     std::printf("\nSSO追加操作统计:\n");
     std::printf("平均吞吐量: %s\n", format_throughput(stats1.mean, buf1, sizeof(buf1)));
     std::printf("标准差: %s\n", format_throughput(stats1.stddev, buf2, sizeof(buf2)));
     std::printf("最小吞吐量: %s\n", format_throughput(stats1.min_value, buf2, sizeof(buf2)));
     std::printf("最大吞吐量: %s\n", format_throughput(stats1.max_value, buf2, sizeof(buf2)));
     std::printf("变异系数: %.2f%%\n", stats1.cv);
     std::printf("平均延迟: %s\n", format_latency(stats1.mean > 0 ? (1e9 / stats1.mean) : 0, buf2, sizeof(buf2)));
     
     // 测试场景2: 超出SSO范围的追加操作 (Append beyond SSO)
     std::printf("\n\n测试场景2: 超出SSO范围追加操作 (长度 > %zu)\n", sso_threshold);
     
     std::vector<double> throughputs2;
     std::vector<double> latencies2;
     
     for (int trial = 1; trial <= trials; ++trial) {
         auto start = high_resolution_clock::now();
         String<> s{};
         
         for (int i = 0; i < iterations; ++i) {
             s.Clear();
             auto st = s.Append(test_data, std::min(test_data_len, static_cast<USize>(sso_threshold + 20)));
         }
         
         auto end = high_resolution_clock::now();
         auto duration = duration_cast<nanoseconds>(end - start).count();
         
         double throughput = (static_cast<double>(iterations) / duration) * 1e9;
         double latency = duration / static_cast<double>(iterations);
         
         throughputs2.push_back(throughput);
         latencies2.push_back(latency);
         
         std::printf("试验 %2d: %s, %s\n", trial,
                    format_throughput(throughput, buf1, sizeof(buf1)),
                    format_latency(latency, buf2, sizeof(buf2)));
     }
     
     auto stats2 = calculate_stats(throughputs2);
     std::printf("\n超出SSO追加操作统计:\n");
     std::printf("平均吞吐量: %s\n", format_throughput(stats2.mean, buf1, sizeof(buf1)));
     std::printf("标准差: %s\n", format_throughput(stats2.stddev, buf2, sizeof(buf2)));
     std::printf("最小吞吐量: %s\n", format_throughput(stats2.min_value, buf2, sizeof(buf2)));
     std::printf("最大吞吐量: %s\n", format_throughput(stats2.max_value, buf2, sizeof(buf2)));
     std::printf("变异系数: %.2f%%\n", stats2.cv);
     std::printf("平均延迟: %s\n", format_latency(stats2.mean > 0 ? (1e9 / stats2.mean) : 0, buf2, sizeof(buf2)));
     
     // 测试场景3: 分配操作 (Assign)
     std::printf("\n\n测试场景3: 分配操作 (Assign)\n");
     
     std::vector<double> throughputs3;
     std::vector<double> latencies3;
     
     for (int trial = 1; trial <= trials; ++trial) {
         auto start = high_resolution_clock::now();
         String<> s{};
         
         for (int i = 0; i < iterations; ++i) {
             auto st = s.Assign(test_data, test_data_len);
         }
         
         auto end = high_resolution_clock::now();
         auto duration = duration_cast<nanoseconds>(end - start).count();
         
         double throughput = (static_cast<double>(iterations) / duration) * 1e9;
         double latency = duration / static_cast<double>(iterations);
         
         throughputs3.push_back(throughput);
         latencies3.push_back(latency);
         
         std::printf("试验 %2d: %s, %s\n", trial,
                    format_throughput(throughput, buf1, sizeof(buf1)),
                    format_latency(latency, buf2, sizeof(buf2)));
     }
     
     auto stats3 = calculate_stats(throughputs3);
     std::printf("\n分配操作统计:\n");
     std::printf("平均吞吐量: %s\n", format_throughput(stats3.mean, buf1, sizeof(buf1)));
     std::printf("标准差: %s\n", format_throughput(stats3.stddev, buf2, sizeof(buf2)));
     std::printf("最小吞吐量: %s\n", format_throughput(stats3.min_value, buf2, sizeof(buf2)));
     std::printf("最大吞吐量: %s\n", format_throughput(stats3.max_value, buf2, sizeof(buf2)));
     std::printf("变异系数: %.2f%%\n", stats3.cv);
     std::printf("平均延迟: %s\n", format_latency(stats3.mean > 0 ? (1e9 / stats3.mean) : 0, buf2, sizeof(buf2)));
     
     // 测试场景4: 清空操作 (Clear)
     std::printf("\n\n测试场景4: 清空操作 (Clear)\n");
     
     std::vector<double> throughputs4;
     std::vector<double> latencies4;
     
     for (int trial = 1; trial <= trials; ++trial) {
         String<> s{};
         s.Assign(test_data, test_data_len);
         
         auto start = high_resolution_clock::now();
         
         for (int i = 0; i < iterations; ++i) {
             s.Clear();
         }
         
         auto end = high_resolution_clock::now();
         auto duration = duration_cast<nanoseconds>(end - start).count();
         
         double throughput = (static_cast<double>(iterations) / duration) * 1e9;
         double latency = duration / static_cast<double>(iterations);
         
         throughputs4.push_back(throughput);
         latencies4.push_back(latency);
         
         std::printf("试验 %2d: %s, %s\n", trial,
                    format_throughput(throughput, buf1, sizeof(buf1)),
                    format_latency(latency, buf2, sizeof(buf2)));
     }
     
     auto stats4 = calculate_stats(throughputs4);
     std::printf("\n清空操作统计:\n");
     std::printf("平均吞吐量: %s\n", format_throughput(stats4.mean, buf1, sizeof(buf1)));
     std::printf("标准差: %s\n", format_throughput(stats4.stddev, buf2, sizeof(buf2)));
     std::printf("最小吞吐量: %s\n", format_throughput(stats4.min_value, buf2, sizeof(buf2)));
     std::printf("最大吞吐量: %s\n", format_throughput(stats4.max_value, buf2, sizeof(buf2)));
     std::printf("变异系数: %.2f%%\n", stats4.cv);
     std::printf("平均延迟: %s\n", format_latency(stats4.mean > 0 ? (1e9 / stats4.mean) : 0, buf2, sizeof(buf2)));
     
     // 测试场景5: 预留容量操作 (Reserve)
     std::printf("\n\n测试场景5: 预留容量操作 (Reserve)\n");
     
     std::vector<double> throughputs5;
     std::vector<double> latencies5;
     
     for (int trial = 1; trial <= trials; ++trial) {
         auto start = high_resolution_clock::now();
         String<> s{};
         
         for (int i = 0; i < iterations; ++i) {
             s.Clear();
             auto st = s.Reserve(sso_threshold * 2);
         }
         
         auto end = high_resolution_clock::now();
         auto duration = duration_cast<nanoseconds>(end - start).count();
         
         double throughput = (static_cast<double>(iterations) / duration) * 1e9;
         double latency = duration / static_cast<double>(iterations);
         
         throughputs5.push_back(throughput);
         latencies5.push_back(latency);
         
         std::printf("试验 %2d: %s, %s\n", trial,
                    format_throughput(throughput, buf1, sizeof(buf1)),
                    format_latency(latency, buf2, sizeof(buf2)));
     }
     
     auto stats5 = calculate_stats(throughputs5);
     std::printf("\n预留容量操作统计:\n");
     std::printf("平均吞吐量: %s\n", format_throughput(stats5.mean, buf1, sizeof(buf1)));
     std::printf("标准差: %s\n", format_throughput(stats5.stddev, buf2, sizeof(buf2)));
     std::printf("最小吞吐量: %s\n", format_throughput(stats5.min_value, buf2, sizeof(buf2)));
     std::printf("最大吞吐量: %s\n", format_throughput(stats5.max_value, buf2, sizeof(buf2)));
     std::printf("变异系数: %.2f%%\n", stats5.cv);
     std::printf("平均延迟: %s\n", format_latency(stats5.mean > 0 ? (1e9 / stats5.mean) : 0, buf2, sizeof(buf2)));
     
     // 综合总结
     std::printf("\n\n=== 测试总结对比 ===\n");
     std::printf("场景                             平均吞吐量          变异系数     稳定性评估\n");
     std::printf("---------------------------------------------------------------------\n");
     std::printf("SSO范围内追加                  %-18s   %6.2f%%    %s\n", 
                format_throughput(stats1.mean, buf1, sizeof(buf1)), stats1.cv, 
                stats1.cv < 5.0 ? "优秀" : (stats1.cv < 10.0 ? "良好" : (stats1.cv < 15.0 ? "一般" : "差")));
     std::printf("超出SSO追加                   %-18s   %6.2f%%    %s\n",
                format_throughput(stats2.mean, buf1, sizeof(buf1)), stats2.cv,
                stats2.cv < 5.0 ? "优秀" : (stats2.cv < 10.0 ? "良好" : (stats2.cv < 15.0 ? "一般" : "差")));
     std::printf("分配操作                      %-18s   %6.2f%%    %s\n",
                format_throughput(stats3.mean, buf1, sizeof(buf1)), stats3.cv,
                stats3.cv < 5.0 ? "优秀" : (stats3.cv < 10.0 ? "良好" : (stats3.cv < 15.0 ? "一般" : "差")));
     std::printf("清空操作                      %-18s   %6.2f%%    %s\n",
                format_throughput(stats4.mean, buf1, sizeof(buf1)), stats4.cv,
                stats4.cv < 5.0 ? "优秀" : (stats4.cv < 10.0 ? "良好" : (stats4.cv < 15.0 ? "一般" : "差")));
     std::printf("预留容量                      %-18s   %6.2f%%    %s\n",
                format_throughput(stats5.mean, buf1, sizeof(buf1)), stats5.cv,
                stats5.cv < 5.0 ? "优秀" : (stats5.cv < 10.0 ? "良好" : (stats5.cv < 15.0 ? "一般" : "差")));
     
     std::printf("\n=== 性能洞察 ===\n");
     std::printf("• SSO范围内操作性能最佳，小字符串优化效果显著\n");
     std::printf("• 超出SSO范围需要堆分配，性能会有明显下降\n");
     std::printf("• 清空操作通常最快，因为不涉及内存分配\n");
     std::printf("• 预留操作涉及内存分配，但比实际数据操作成本低\n");
     
     return 0;
 }