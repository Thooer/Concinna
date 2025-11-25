/**
 * @file PerformanceTest.cppm
 * @brief Test Framework Performance and Memory Testing Example
 * @details
 * - 演示性能测试功能
 * - 展示内存泄漏检测
 * - 对比不同算法的性能
 * - 展示基准测试框架
 */
module;

#include "TestMacros.hpp"

export module PerformanceTest;

import Language;
import Language:Error;
import Test;
import Test:Context;
import Test:Assertions;
import Test:Runner;

export namespace PerformanceTest {

    // 简单数据结构用于性能测试
    struct PerformanceData {
        int* data{nullptr};
        int size{0};

        PerformanceData() = default;
        explicit PerformanceData(int n) : size(n) {
            data = new int[size];
            for (int i = 0; i < size; ++i) {
                data[i] = i;
            }
        }

        ~PerformanceData() {
            delete[] data;
        }

        // 禁止拷贝
        PerformanceData(const PerformanceData&) = delete;
        PerformanceData& operator=(const PerformanceData&) = delete;

        // 允许移动
        PerformanceData(PerformanceData&& other) noexcept : data(other.data), size(other.size) {
            other.data = nullptr;
            other.size = 0;
        }

        PerformanceData& operator=(PerformanceData&& other) noexcept {
            if (this != &other) {
                delete[] data;
                data = other.data;
                size = other.size;
                other.data = nullptr;
                other.size = 0;
            }
            return *this;
        }
    };

    // 算法实现用于性能对比
    namespace Algorithms {
        // 冒泡排序
        void BubbleSort(int* arr, int size) {
            for (int i = 0; i < size - 1; ++i) {
                for (int j = 0; j < size - i - 1; ++j) {
                    if (arr[j] > arr[j + 1]) {
                        std::swap(arr[j], arr[j + 1]);
                    }
                }
            }
        }

        // 快速排序
        void QuickSort(int* arr, int low, int high) {
            if (low < high) {
                int pivot = arr[high];
                int i = low - 1;

                for (int j = low; j < high; ++j) {
                    if (arr[j] <= pivot) {
                        ++i;
                        std::swap(arr[i], arr[j]);
                    }
                }
                std::swap(arr[i + 1], arr[high]);
                int pi = i + 1;

                QuickSort(arr, low, pi - 1);
                QuickSort(arr, pi + 1, high);
            }
        }

        void QuickSort(int* arr, int size) {
            if (size > 0) {
                QuickSort(arr, 0, size - 1);
            }
        }

        // 验证数组是否已排序
        bool IsSorted(const int* arr, int size) {
            for (int i = 1; i < size; ++i) {
                if (arr[i - 1] > arr[i]) {
                    return false;
                }
            }
            return true;
        }
    }

    // 基准测试类
    class Benchmark {
    public:
        Benchmark(const char* name) : m_name(name) {}

        template<typename Func>
        Benchmark& Run(const char* testName, Func&& func, int iterations = 1000) {
            m_tests.push_back({testName, std::forward<Func>(func), iterations});
            return *this;
        }

        void Execute(Test::Context& context) {
            context.GetLogger().Info(std::format("Executing benchmark: {}", m_name).c_str());

            for (const auto& test : m_tests) {
                context.GetTimer().Start();
                
                for (int i = 0; i < test.iterations; ++i) {
                    test.func();
                }
                
                context.GetTimer().Stop();
                double totalTime = context.GetTimer().GetElapsedTimeMs();
                double avgTime = totalTime / test.iterations;

                context.GetLogger().Info(std::format(
                    "  {}: {:.3f}ms (avg over {} iterations)", 
                    test.name, avgTime, test.iterations
                ).c_str());
            }
        }

    private:
        struct TestCase {
            const char* name;
            std::function<void()> func;
            int iterations;
        };

        std::string m_name;
        std::vector<TestCase> m_tests;
    };

    // 内存泄漏检测测试
    class MemoryLeakTest : public Test::ITest {
    public:
        MemoryLeakTest() : Test::ITest("MemoryLeakTest") {}

        void TestMemoryAllocation() {
            Test::Context ctx = Test::DefaultContextFactory::Create();
            
            // 启动内存跟踪
            ctx.GetMemoryTracker().StartTracking();
            ctx.GetLogger().Info("Starting memory allocation test");

            // 执行多次分配和释放
            constexpr int numAllocations = 100;
            std::vector<void*> allocations;

            for (int i = 0; i < numAllocations; ++i) {
                size_t size = (i + 1) * 16; // 16, 32, 48, ...
                void* ptr = malloc(size);
                allocations.push_back(ptr);
                
                // 记录分配
                ctx.GetMemoryTracker().RecordAllocation(ptr, size, 8, __FILE__, __LINE__, "test_allocation");
            }

            ctx.GetLogger().Info("Performing partial deallocations");

            // 故意泄漏一些内存进行演示
            for (int i = 0; i < numAllocations / 2; ++i) {
                void* ptr = allocations[i];
                ctx.GetMemoryTracker().RecordDeallocation(ptr);
                free(ptr);
            }

            // 检查泄漏
            ctx.GetMemoryTracker().StopTracking();
            
            UInt64 leakedCount = ctx.GetMemoryTracker().GetLeakedCount();
            UInt64 leakedSize = ctx.GetMemoryTracker().GetLeakedSize();

            ctx.GetLogger().Info(std::format(
                "Memory test completed. Leaked: {} allocations, {} bytes",
                leakedCount, leakedSize
            ).c_str());

            // 验证预期的泄漏（演示目的）
            Test::Assert::Equal(ctx, static_cast<UInt64>(numAllocations / 2), leakedCount, 
                              "Should have exactly half the allocations leaked");
            
            if (leakedCount > 0) {
                ctx.GetMemoryTracker().GenerateLeakReport();
            }
        }

        void Run() override {
            TestMemoryAllocation();
        }

        [[nodiscard]] const char* GetName() const noexcept override { return m_name.c_str(); }
        [[nodiscard]] Test::TestResult GetResult() const noexcept override { return m_result; }
        void SetResult(Test::TestResult result) noexcept override { m_result = result; }

    private:
        std::string m_name{"MemoryLeakTest"};
        Test::TestResult m_result{Test::TestResult::NotRun};
    };

    // 性能对比测试
    class PerformanceComparisonTest : public Test::ITest {
    public:
        PerformanceComparisonTest() : Test::ITest("PerformanceComparisonTest") {}

        void TestSortingAlgorithms() {
            Test::Context ctx = Test::DefaultContextFactory::Create();
            
            ctx.GetLogger().Info("Starting sorting algorithm performance comparison");
            
            // 创建测试数据
            constexpr int dataSize = 1000;
            std::vector<int> testData(dataSize);
            
            // 填充随机数据
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1, 10000);
            
            for (int& value : testData) {
                value = dis(gen);
            }

            // 准备算法测试数据
            std::vector<int> bubbleData = testData;
            std::vector<int> quickData = testData;

            // 基准测试
            Benchmark("Sorting Algorithms Performance")
                .Run("Bubble Sort", [&]() {
                    std::vector<int> data = testData;
                    Algorithms::BubbleSort(data.data(), static_cast<int>(data.size()));
                    Test::Assert::True(ctx, Algorithms::IsSorted(data.data(), static_cast<int>(data.size())), "Bubble sort should produce sorted array");
                }, 100)

                .Run("Quick Sort", [&]() {
                    std::vector<int> data = testData;
                    Algorithms::QuickSort(data.data(), static_cast<int>(data.size()));
                    Test::Assert::True(ctx, Algorithms::IsSorted(data.data(), static_cast<int>(data.size())), "Quick sort should produce sorted array");
                }, 100)

                .Run("std::sort", [&]() {
                    std::vector<int> data = testData;
                    std::sort(data.begin(), data.end());
                    Test::Assert::True(ctx, std::is_sorted(data.begin(), data.end()), "std::sort should produce sorted array");
                }, 100)

                .Execute(ctx);
        }

        void TestMemoryOperations() {
            Test::Context ctx = Test::DefaultContextFactory::Create();
            ctx.GetMemoryTracker().StartTracking();

            Benchmark("Memory Operations Performance")
                .Run("Stack Allocation", [&]() {
                    int arr[100];
                    for (int i = 0; i < 100; ++i) {
                        arr[i] = i;
                    }
                }, 10000)

                .Run("Heap Allocation", [&]() {
                    int* arr = new int[100];
                    for (int i = 0; i < 100; ++i) {
                        arr[i] = i;
                    }
                    delete[] arr;
                }, 1000)

                .Run("Object Construction", [&]() {
                    PerformanceData data(100);
                    for (int i = 0; i < 100; ++i) {
                        data.data[i] = i;
                    }
                }, 5000)

                .Execute(ctx);

            ctx.GetMemoryTracker().StopTracking();
            
            // 验证内存操作无泄漏
            Test::Assert::False(ctx, ctx.GetMemoryTracker().HasLeaks(), "Memory operations should not leak");
        }

        void Run() override {
            TestSortingAlgorithms();
            TestMemoryOperations();
        }

        [[nodiscard]] const char* GetName() const noexcept override { return m_name.c_str(); }
        [[nodiscard]] Test::TestResult GetResult() const noexcept override { return m_result; }
        void SetResult(Test::TestResult result) noexcept override { m_result = result; }

    private:
        std::string m_name{"PerformanceComparisonTest"};
        Test::TestResult m_result{Test::TestResult::NotRun};
    };

    // 微基准测试示例
    class MicroBenchmarkTest : public Test::ITest {
    public:
        MicroBenchmarkTest() : Test::ITest("MicroBenchmarkTest") {}

        void TestArithmeticOperations() {
            Test::Context ctx = Test::DefaultContextFactory::Create();

            Benchmark("Arithmetic Operations")
                .Run("Integer Addition", [&]() {
                    volatile int result = 0;
                    for (int i = 0; i < 1000; ++i) {
                        result += i;
                    }
                }, 10000)

                .Run("Integer Multiplication", [&]() {
                    volatile int result = 1;
                    for (int i = 1; i < 100; ++i) {
                        result *= i;
                    }
                }, 10000)

                .Run("Floating Point Division", [&]() {
                    volatile double result = 1.0;
                    for (int i = 1; i < 1000; ++i) {
                        result /= i;
                    }
                }, 1000)

                .Execute(ctx);
        }

        void TestStringOperations() {
            Test::Context ctx = Test::DefaultContextFactory::Create();

            Benchmark("String Operations")
                .Run("String Concatenation", [&]() {
                    std::string result;
                    for (int i = 0; i < 100; ++i) {
                        result += std::to_string(i);
                    }
                }, 1000)

                .Run("String Comparison", [&]() {
                    std::string str1 = "Hello World";
                    std::string str2 = "Hello World";
                    volatile bool result = (str1 == str2);
                }, 10000)

                .Run("String Find", [&]() {
                    std::string str = "The quick brown fox jumps over the lazy dog";
                    volatile size_t pos = str.find("fox");
                }, 1000)

                .Execute(ctx);
        }

        void Run() override {
            TestArithmeticOperations();
            TestStringOperations();
        }

        [[nodiscard]] const char* GetName() const noexcept override { return m_name.c_str(); }
        [[nodiscard]] Test::TestResult GetResult() const noexcept override { return m_result; }
        void SetResult(Test::TestResult result) noexcept override { m_result = result; }

    private:
        std::string m_name{"MicroBenchmarkTest"};
        Test::TestResult m_result{Test::TestResult::NotRun};
    };
}

// 主测试函数
export int main() {
    auto logger = Test::ContextUtils::CreateDefaultLogger(Test::LogLevel::Info);
    auto timer = Test::ContextUtils::CreateDefaultTimer();
    auto allocator = Test::ContextUtils::CreateDefaultAllocator(16 * 1024);
    auto memoryTracker = Test::ContextUtils::CreateDefaultMemoryTracker(allocator);

    Test::Context context(logger, timer, allocator, memoryTracker);
    Test::TestRunner runner(context);

    // 添加性能测试用例
    runner.AddTest(std::make_unique<PerformanceTest::MemoryLeakTest>());
    runner.AddTest(std::make_unique<PerformanceTest::PerformanceComparisonTest>());
    runner.AddTest(std::make_unique<PerformanceTest::MicroBenchmarkTest>());

    // 配置运行器
    runner.SetFilter("Performance*"); // 运行所有性能测试
    runner.SetParallel(true);
    runner.SetTimeout(10000); // 10秒超时（性能测试可能需要更长时间）

    context.GetLogger().Info("Starting Performance Test Suite");
    auto result = runner.Run();

    context.GetLogger().Info("Performance test suite completed");
    Test::Output::Console::PrintResults(context, result);
    Test::Output::Console::PrintStatistics(context, runner.GetStatistics());

    return result.IsSuccessful() ? 0 : 1;
}