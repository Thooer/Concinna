/**
 * @file TestMacros.hpp
 * @brief Test Framework Macros - 测试框架宏定义
 * @details
 * - 提供便捷的测试宏定义
 * - 简化断言和测试用例编写
 * - 支持条件断言和异常检测
 */

#pragma once

// 基础测试宏
#define TEST(TEST_CLASS_NAME, TEST_METHOD_NAME) \
    void TEST_METHOD_NAME() override

#define TEST_CASE(TEST_METHOD_NAME) \
    void TEST_METHOD_NAME() 

// 基础断言宏
#define ASSERT_TRUE(context, condition, message) \
    Test::Assert::True(context, condition, message)

#define ASSERT_FALSE(context, condition, message) \
    Test::Assert::False(context, condition, message)

#define ASSERT_EQ(context, expected, actual, message) \
    Test::Assert::Equal(context, expected, actual, message)

#define ASSERT_NE(context, expected, actual, message) \
    Test::Assert::NotEqual(context, expected, actual, message)

#define ASSERT_LT(context, left, right, message) \
    Test::Assert::Less(context, left, right, message)

#define ASSERT_LE(context, left, right, message) \
    Test::Assert::LessEqual(context, left, right, message)

#define ASSERT_GT(context, left, right, message) \
    Test::Assert::Greater(context, left, right, message)

#define ASSERT_GE(context, left, right, message) \
    Test::Assert::GreaterEqual(context, left, right, message)

// 浮点断言宏
#define ASSERT_FLOAT_EQ(context, expected, actual, tolerance, message) \
    Test::Assert::FloatEqual(context, expected, actual, tolerance, message)

#define ASSERT_DOUBLE_EQ(context, expected, actual, tolerance, message) \
    Test::Assert::DoubleEqual(context, expected, actual, tolerance, message)

// 指针断言宏
#define ASSERT_NULL(context, pointer, message) \
    Test::Assert::Null(context, pointer, message)

#define ASSERT_NOT_NULL(context, pointer, message) \
    Test::Assert::NotNull(context, pointer, message)

// 字符串断言宏
#define ASSERT_STREQ(context, expected, actual, message) \
    Test::Assert::StringEqual(context, expected, actual, message)

#define ASSERT_STRNE(context, expected, actual, message) \
    Test::Assert::StringNotEqual(context, expected, actual, message)

// 容器断言宏
#define ASSERT_CONTAINER_EQ(context, expected, actual, message) \
    Test::Assert::ContainerEqual(context, expected, actual, message)

#define ASSERT_CONTAINER_NE(context, expected, actual, message) \
    Test::Assert::ContainerNotEqual(context, expected, actual, message)

#define ASSERT_EMPTY(context, container, message) \
    Test::Assert::Empty(context, container, message)

#define ASSERT_NOT_EMPTY(context, container, message) \
    Test::Assert::NotEmpty(context, container, message)

#define ASSERT_CONTAINS(context, container, element, message) \
    Test::Assert::Contains(context, container, element, message)

#define ASSERT_NOT_CONTAINS(context, container, element, message) \
    Test::Assert::NotContains(context, container, element, message)

// 数学向量断言宏
#define ASSERT_VECTOR3F_EQ(context, expected, actual, tolerance, message) \
    Test::Assert::Vector3fEqual(context, expected, actual, tolerance, message)

#define ASSERT_VECTOR2F_EQ(context, expected, actual, tolerance, message) \
    Test::Assert::Vector2fEqual(context, expected, actual, tolerance, message)

#define ASSERT_QUAT_EQ(context, expected, actual, tolerance, message) \
    Test::Assert::QuaternionEqual(context, expected, actual, tolerance, message)

#define ASSERT_MATRIX_EQ(context, expected, actual, tolerance, message) \
    Test::Assert::MatrixEqual(context, expected, actual, tolerance, message)

// 异常断言宏
#define ASSERT_THROWS(context, exceptionType, func, message) \
    Test::Assert::Throws<exceptionType>(context, func, message)

#define ASSERT_THROWS_ANY(context, func, message) \
    Test::Assert::ThrowsAny(context, func, message)

#define ASSERT_NO_THROW(context, func, message) \
    Test::Assert::NoThrow(context, func, message)

// 条件断言宏
#define EXPECT_TRUE(context, condition, message) \
    Test::Assert::ExpectTrue(context, condition, message)

#define EXPECT_FALSE(context, condition, message) \
    Test::Assert::ExpectFalse(context, condition, message)

#define EXPECT_EQ(context, expected, actual, message) \
    Test::Assert::ExpectEqual(context, expected, actual, message)

#define EXPECT_NE(context, expected, actual, message) \
    Test::Assert::ExpectNotEqual(context, expected, actual, message)

// 性能测试宏
#define BENCHMARK(context, name, iterations) \
    Test::Benchmark(name, iterations, context.GetTimer(), context.GetLogger())

// 内存跟踪宏
#define MEMORY_TRACK_START(context) \
    context.GetMemoryTracker().StartTracking()

#define MEMORY_TRACK_STOP(context) \
    context.GetMemoryTracker().StopTracking()

#define ALLOCATE_TEST(context, size, alignment, tag) \
    (context.GetMemoryTracker().RecordAllocation( \
        context.GetAllocator().Allocate(size, alignment), size, alignment, \
        __FILE__, __LINE__, tag))

// 日志记录宏
#define LOG_INFO(context, message) \
    context.GetLogger().Info(message)

#define LOG_DEBUG(context, message) \
    context.GetLogger().Debug(message)

#define LOG_WARNING(context, message) \
    context.GetLogger().Warning(message)

#define LOG_ERROR(context, message) \
    context.GetLogger().Error(message)

#define LOG_FORMAT(context, level, format, ...) \
    context.GetLogger().LogFormat(level, format, __VA_ARGS__)

// 测试套件宏
#define TEST_SUITE(SUITE_NAME) \
    class SUITE_NAME : public Test::ITest { \
    public: \
        SUITE_NAME() : Test::ITest(#SUITE_NAME) {} \
        void Run() override

#define END_TEST_SUITE() \
    }; 

// 快速测试定义宏
#define SIMPLE_TEST(TEST_NAME, TEST_BODY) \
    class TEST_NAME : public Test::ITest { \
    public: \
        TEST_NAME() : Test::ITest(#TEST_NAME) {} \
        void Run() override { \
            Test::Context ctx = Test::DefaultContextFactory::Create(); \
            TEST_BODY \
        } \
    private: \
        Test::TestResult m_result{Test::TestResult::NotRun}; \
    public: \
        [[nodiscard]] const char* GetName() const noexcept override { return m_name.c_str(); } \
        [[nodiscard]] Test::TestResult GetResult() const noexcept override { return m_result; } \
        void SetResult(Test::TestResult result) noexcept override { m_result = result; } \
    private: \
        std::string m_name{#TEST_NAME}; \
    };

// 禁用复制构造和赋值操作符的宏（防止误用）
#define NO_COPY(class_name) \
    class_name(const class_name&) = delete; \
    class_name& operator=(const class_name&) = delete

// 启用移动构造和赋值的宏
#define MOVE_ONLY(class_name) \
    class_name(class_name&&) noexcept = default; \
    class_name& operator=(class_name&&) noexcept = default

// 禁用特定构造函数的宏
#define DELETE_CTOR(class_name, ctor_type) \
    class_name ctor_type = delete

// 自定义断言宏（允许更复杂的断言逻辑）
#define CUSTOM_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            throw Test::AssertionFailedException(message); \
        } \
    } while (false)

// 条件编译测试宏
#if defined(_DEBUG) || defined(DEBUG)
    #define DEBUG_TEST_ONLY(code) code
    #define RELEASE_TEST_ONLY(code)
#else
    #define DEBUG_TEST_ONLY(code)
    #define RELEASE_TEST_ONLY(code) code
#endif

// 平台相关的测试宏
#if defined(_WIN32)
    #define PLATFORM_WINDOWS() 1
    #define PLATFORM_LINUX() 0
    #define PLATFORM_MACOS() 0
#elif defined(__linux__)
    #define PLATFORM_WINDOWS() 0
    #define PLATFORM_LINUX() 1
    #define PLATFORM_MACOS() 0
#elif defined(__APPLE__)
    #define PLATFORM_WINDOWS() 0
    #define PLATFORM_LINUX() 0
    #define PLATFORM_MACOS() 1
#else
    #define PLATFORM_WINDOWS() 0
    #define PLATFORM_LINUX() 0
    #define PLATFORM_MACOS() 0
#endif

// 编译器特性检测宏
#if defined(__GNUC__) || defined(__clang__)
    #define COMPILER_CLANG_GCC() 1
    #define COMPILER_MSVC() 0
#elif defined(_MSC_VER)
    #define COMPILER_CLANG_GCC() 0
    #define COMPILER_MSVC() 1
#else
    #define COMPILER_CLANG_GCC() 0
    #define COMPILER_MSVC() 0
#endif

// 架构相关的测试宏
#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__)
    #define ARCHITECTURE_X64() 1
    #define ARCHITECTURE_X86() 0
    #define ARCHITECTURE_ARM() 0
#elif defined(_M_IX86) || defined(__i386__)
    #define ARCHITECTURE_X64() 0
    #define ARCHITECTURE_X86() 1
    #define ARCHITECTURE_ARM() 0
#elif defined(_M_ARM) || defined(__arm__) || defined(__aarch64__)
    #define ARCHITECTURE_X64() 0
    #define ARCHITECTURE_X86() 0
    #define ARCHITECTURE_ARM() 1
#else
    #define ARCHITECTURE_X64() 0
    #define ARCHITECTURE_X86() 0
    #define ARCHITECTURE_ARM() 0
#endif

// 便捷宏，用于快速创建测试函数
#define RUN_TEST(test_class, context) \
    do { \
        test_class test; \
        try { \
            test.Run(); \
        } catch (const std::exception& e) { \
            Test::Assert::Fail(context, e.what(), "Uncaught exception in test"); \
        } catch (...) { \
            Test::Assert::Fail(context, "Unknown exception", "Uncaught unknown exception in test"); \
        } \
    } while (false)

#define TEST_AUTO_REGISTER(category, name, fn) static ::Test::AutoRegister s_auto_##name(category, name, fn, false)
#define TEST_AUTO_REGISTER_FRAME(category, name, fn) static ::Test::AutoRegister s_auto_##name##_frame(category, name, fn, true)