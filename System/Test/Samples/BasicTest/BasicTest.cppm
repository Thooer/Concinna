/**
 * @file BasicTest.cppm
 * @brief Test Framework Basic Usage Example
 * @details
 * - 演示基础测试框架使用方法
 * - 包含基本断言、异常测试、上下文使用示例
 * - 展示测试运行器配置和执行
 */
module;

#include "TestMacros.hpp"

export module BasicTest;

import Language;
import Language:Error;
import Test;
import Test:Context;
import Test:Assertions;
import Test:Runner;

export namespace BasicTest {

    // 基础数学工具函数用于测试
    class MathUtils {
    public:
        [[nodiscard]] static Int32 Add(Int32 a, Int32 b) noexcept { return a + b; }
        [[nodiscard]] static Int32 Divide(Int32 a, Int32 b) {
            if (b == 0) {
                throw std::runtime_error("Division by zero");
            }
            return a / b;
        }
        [[nodiscard]] static bool IsEven(Int32 value) noexcept { return value % 2 == 0; }
        [[nodiscard]] static float SquareRoot(float value) {
            if (value < 0.0f) {
                throw std::invalid_argument("Cannot calculate square root of negative number");
            }
            return std::sqrt(value);
        }
    };

    // 基础测试类示例
    class MathUtilsTests : public Test::ITest {
    public:
        MathUtilsTests() : Test::ITest("MathUtilsTests") {}

        // 测试加法
        void TestAddition() {
            Test::Context ctx = Test::DefaultContextFactory::Create();
            
            // 基本断言测试
            Test::Assert::Equal(ctx, 5, MathUtils::Add(2, 3), "2 + 3 should equal 5");
            Test::Assert::Equal(ctx, 0, MathUtils::Add(-5, 5), "-5 + 5 should equal 0");
            Test::Assert::Equal(ctx, -10, MathUtils::Add(-5, -5), "-5 + -5 should equal -10");

            // 边界值测试
            Test::Assert::Equal(ctx, INT32_MAX, MathUtils::Add(INT32_MAX, 0), "INT32_MAX + 0 should equal INT32_MAX");
        }

        // 测试除法
        void TestDivision() {
            Test::Context ctx = Test::DefaultContextFactory::Create();
            
            Test::Assert::Equal(ctx, 2, MathUtils::Divide(10, 5), "10 / 5 should equal 2");
            Test::Assert::Equal(ctx, -2, MathUtils::Divide(-10, 5), "-10 / 5 should equal -2");
            Test::Assert::Equal(ctx, 0, MathUtils::Divide(0, 5), "0 / 5 should equal 0");

            // 异常测试
            Test::Assert::Throws<decltype(std::runtime_error("test"))>(ctx, 
                []() { MathUtils::Divide(10, 0); },
                "Division by zero should throw runtime_error");
        }

        // 测试偶数判断
        void TestIsEven() {
            Test::Context ctx = Test::DefaultContextFactory::Create();
            
            Test::Assert::True(ctx, MathUtils::IsEven(2), "2 is even");
            Test::Assert::True(ctx, MathUtils::IsEven(0), "0 is even");
            Test::Assert::True(ctx, MathUtils::IsEven(-2), "-2 is even");
            Test::Assert::False(ctx, MathUtils::IsEven(1), "1 is not even");
            Test::Assert::False(ctx, MathUtils::IsEven(-1), "-1 is not even");
        }

        // 测试平方根
        void TestSquareRoot() {
            Test::Context ctx = Test::DefaultContextFactory::Create();
            
            Test::Assert::FloatEqual(ctx, 2.0f, MathUtils::SquareRoot(4.0f), 0.001f, "sqrt(4) should equal 2.0");
            Test::Assert::FloatEqual(ctx, 0.0f, MathUtils::SquareRoot(0.0f), 0.001f, "sqrt(0) should equal 0.0");
            Test::Assert::FloatEqual(ctx, 1.41421356f, MathUtils::SquareRoot(2.0f), 0.001f, "sqrt(2) should approximately equal 1.414");

            // 异常测试
            Test::Assert::Throws<decltype(std::invalid_argument("test"))>(ctx,
                []() { MathUtils::SquareRoot(-1.0f); },
                "Square root of negative should throw invalid_argument");
        }

        // 实现 ITest 接口
        void Run() override {
            TestAddition();
            TestDivision();
            TestIsEven();
            TestSquareRoot();
        }

        [[nodiscard]] const char* GetName() const noexcept override { return m_name.c_str(); }
        [[nodiscard]] Test::TestResult GetResult() const noexcept override { return m_result; }
        void SetResult(Test::TestResult result) noexcept override { m_result = result; }

    private:
        std::string m_name{"MathUtilsTests"};
        Test::TestResult m_result{Test::TestResult::NotRun};
    };

    // 上下文使用示例
    class ContextUsageExample : public Test::ITest {
    public:
        ContextUsageExample() : Test::ITest("ContextUsageExample") {}

        void TestContextFeatures() {
            auto logger = Test::ContextUtils::CreateDefaultLogger(Test::LogLevel::Debug);
            auto timer = Test::ContextUtils::CreateDefaultTimer();
            auto allocator = Test::ContextUtils::CreateDefaultAllocator(1024);
            auto memoryTracker = Test::ContextUtils::CreateDefaultMemoryTracker(allocator);

            Test::Context ctx(logger, timer, allocator, memoryTracker);

            // 记录开始
            ctx.GetLogger().Info("Starting context usage example");

            // 内存分配测试
            ctx.GetMemoryTracker().StartTracking();
            
            void* ptr = ctx.GetAllocator().Allocate(256);
            ctx.GetLogger().Debug("Allocated 256 bytes");
            ctx.GetMemoryTracker().RecordAllocation(ptr, 256, 16, __FILE__, __LINE__, "test_allocation");
            
            // 模拟一些使用
            std::memset(ptr, 0xFF, 256);
            
            // 释放内存
            ctx.GetMemoryTracker().RecordDeallocation(ptr);
            ctx.GetAllocator().Deallocate(ptr);
            ctx.GetLogger().Debug("Deallocated memory");

            ctx.GetMemoryTracker().StopTracking();

            // 性能测试
            timer.Start();
            volatile int sum = 0;
            for (int i = 0; i < 1000; ++i) {
                sum += MathUtils::Add(i, i + 1);
            }
            timer.Stop();

            ctx.GetLogger().Info("Context features test completed");
            Test::Assert::True(ctx, sum > 0, "Performance test should produce non-zero result");
        }

        void Run() override {
            TestContextFeatures();
        }

        [[nodiscard]] const char* GetName() const noexcept override { return m_name.c_str(); }
        [[nodiscard]] Test::TestResult GetResult() const noexcept override { return m_result; }
        void SetResult(Test::TestResult result) noexcept override { m_result = result; }

    private:
        std::string m_name{"ContextUsageExample"};
        Test::TestResult m_result{Test::TestResult::NotRun};
    };

    // 高级断言示例
    class AdvancedAssertionsExample : public Test::ITest {
    public:
        AdvancedAssertionsExample() : Test::ITest("AdvancedAssertionsExample") {}

        void TestVectorAssertions() {
            Test::Context ctx = Test::DefaultContextFactory::Create();
            
            // 向量断言测试
            Test::Math::Vector3f vec1{1.0f, 2.0f, 3.0f};
            Test::Math::Vector3f vec2{1.0f, 2.0f, 3.0f};
            Test::Math::Vector3f vec3{4.0f, 5.0f, 6.0f};

            Test::Assert::Vector3fEqual(ctx, vec1, vec2, 0.001f, "Equal vectors should be equal");
            Test::Assert::Vector3fNotEqual(ctx, vec1, vec3, 0.001f, "Different vectors should not be equal");
            
            // 向量运算测试
            Test::Math::Vector3f result = vec1 + vec3;
            Test::Math::Vector3f expected{5.0f, 7.0f, 9.0f};
            Test::Assert::Vector3fEqual(ctx, result, expected, 0.001f, "Vector addition should work correctly");
        }

        void TestContainerAssertions() {
            Test::Context ctx = Test::DefaultContextFactory::Create();
            
            // 容器断言测试
            std::vector<int> vec1{1, 2, 3, 4, 5};
            std::vector<int> vec2{1, 2, 3, 4, 5};
            std::vector<int> vec3{5, 4, 3, 2, 1};

            Test::Assert::ContainerEqual(ctx, vec1, vec2, "Equal containers should be equal");
            Test::Assert::ContainerNotEqual(ctx, vec1, vec3, "Different containers should not be equal");

            // 空容器测试
            std::vector<int> emptyVec;
            std::vector<int> nonEmptyVec{1};

            Test::Assert::Empty(ctx, emptyVec, "Empty vector should be empty");
            Test::Assert::NotEmpty(ctx, nonEmptyVec, "Non-empty vector should not be empty");

            // 包含测试
            Test::Assert::Contains(ctx, vec1, 3, "Vector should contain 3");
            Test::Assert::NotContains(ctx, vec1, 10, "Vector should not contain 10");
        }

        void Run() override {
            TestVectorAssertions();
            TestContainerAssertions();
        }

        [[nodiscard]] const char* GetName() const noexcept override { return m_name.c_str(); }
        [[nodiscard]] Test::TestResult GetResult() const noexcept override { return m_result; }
        void SetResult(Test::TestResult result) noexcept override { m_result = result; }

    private:
        std::string m_name{"AdvancedAssertionsExample"};
        Test::TestResult m_result{Test::TestResult::NotRun};
    };
}

// 主测试函数
export int main() {
    // 创建测试运行器
    auto logger = Test::ContextUtils::CreateDefaultLogger(Test::LogLevel::Info);
    auto timer = Test::ContextUtils::CreateDefaultTimer();
    auto allocator = Test::ContextUtils::CreateDefaultAllocator(8192);
    auto memoryTracker = Test::ContextUtils::CreateDefaultMemoryTracker(allocator);

    Test::Context context(logger, timer, allocator, memoryTracker);
    Test::TestRunner runner(context);

    // 添加测试用例
    runner.AddTest(std::make_unique<BasicTest::MathUtilsTests>());
    runner.AddTest(std::make_unique<BasicTest::ContextUsageExample>());
    runner.AddTest(std::make_unique<BasicTest::AdvancedAssertionsExample>());

    // 配置运行器
    runner.SetFilter("*"); // 运行所有测试
    runner.SetParallel(true); // 启用并行执行
    runner.SetTimeout(5000); // 5秒超时

    // 运行测试
    context.GetLogger().Info("Starting Basic Test Suite");
    auto result = runner.Run();

    // 报告结果
    context.GetLogger().Info("Test suite completed");
    Test::Output::Console::PrintResults(context, result);
    Test::Output::Console::PrintStatistics(context, runner.GetStatistics());

    return result.IsSuccessful() ? 0 : 1;
}