module;

export module Test:Core;

import Language;

export namespace Test {

    inline constexpr StatusDomain TestDomain{"Test", 0, -1};

    enum class TestState : UInt8 {
        Pending,
        Running,
        Completed,
        Failed,
        Skipped,
        TimedOut
    };

    struct TestResult {
        enum class Kind : UInt8 {
            Success,
            Failure,
            Skipped,
            Timeout
        };

        Kind kind{Kind::Success};
        const char* message{nullptr};
        const char* file{nullptr};
        Int32 line{0};

        [[nodiscard]] constexpr bool IsSuccess() const noexcept { return kind == Kind::Success; }
        [[nodiscard]] constexpr bool IsFailure() const noexcept { return kind == Kind::Failure; }
        [[nodiscard]] constexpr bool IsSkipped() const noexcept { return kind == Kind::Skipped; }
        [[nodiscard]] constexpr bool IsTimeout() const noexcept { return kind == Kind::Timeout; }
    };

    struct TestStatistics {
        UInt64 totalTests{0};
        UInt64 passedTests{0};
        UInt64 failedTests{0};
        UInt64 skippedTests{0};
        UInt64 timedOutTests{0};
        Float64 totalTimeMs{0.0};
        Float64 averageTimeMs{0.0};
    };

    class ITestLogger;
    class ITestTimer;
    class ITestAllocator;
    class ITestMemoryTracker;
    struct TestResultBuilder;

    class ITestContext {
    public:
        virtual ~ITestContext() = default;

        [[nodiscard]] virtual ITestLogger& Logger() noexcept = 0;
        [[nodiscard]] virtual const ITestLogger& Logger() const noexcept = 0;

        [[nodiscard]] virtual ITestTimer& Timer() noexcept = 0;
        [[nodiscard]] virtual const ITestTimer& Timer() const noexcept = 0;

        [[nodiscard]] virtual ITestAllocator& Allocator() noexcept = 0;
        [[nodiscard]] virtual const ITestAllocator& Allocator() const noexcept = 0;
        [[nodiscard]] virtual ITestMemoryTracker& MemoryTracker() noexcept = 0;
        [[nodiscard]] virtual const ITestMemoryTracker& MemoryTracker() const noexcept = 0;

        [[nodiscard]] virtual TestState GetState() const noexcept = 0;
        virtual void SetState(TestState state) noexcept = 0;

        virtual void SetResult(const TestResult& result) noexcept = 0;
        [[nodiscard]] virtual const TestResult& GetResult() const noexcept = 0;

        virtual void PushNote(const char* message) noexcept = 0;
        [[nodiscard]] virtual const char* LastNote() const noexcept = 0;

        virtual void Yield(UInt32 /*frames*/) noexcept { }
        virtual void WaitUntil(bool(*)(void) noexcept) noexcept { }
        virtual void Suspend() noexcept { }
        virtual bool Resume() noexcept { return true; }
        [[nodiscard]] virtual UInt32 GetCurrentFrame() const noexcept { return 0; }
        [[nodiscard]] virtual Float64 GetDeltaTime() const noexcept { return 0.0; }
    };

    class TestContext final : public ITestContext {
    public:
        TestContext(ITestLogger& logger,
                    ITestTimer& timer,
                    ITestAllocator& allocator,
                    ITestMemoryTracker& tracker) noexcept;

        [[nodiscard]] ITestLogger& Logger() noexcept override { return *m_logger; }
        [[nodiscard]] const ITestLogger& Logger() const noexcept override { return *m_logger; }

        [[nodiscard]] ITestTimer& Timer() noexcept override { return *m_timer; }
        [[nodiscard]] const ITestTimer& Timer() const noexcept override { return *m_timer; }

        [[nodiscard]] ITestAllocator& Allocator() noexcept override { return *m_allocator; }
        [[nodiscard]] const ITestAllocator& Allocator() const noexcept override { return *m_allocator; }

        [[nodiscard]] ITestMemoryTracker& MemoryTracker() noexcept override { return *m_tracker; }
        [[nodiscard]] const ITestMemoryTracker& MemoryTracker() const noexcept override { return *m_tracker; }

        [[nodiscard]] TestState GetState() const noexcept override { return m_state; }
        void SetState(TestState state) noexcept override { m_state = state; }

        void SetResult(const TestResult& result) noexcept override { m_result = result; }
        [[nodiscard]] const TestResult& GetResult() const noexcept override { return m_result; }

        void PushNote(const char* message) noexcept override { m_lastNote = message; }
        [[nodiscard]] const char* LastNote() const noexcept override { return m_lastNote; }

    private:
        ITestLogger* m_logger{nullptr};
        ITestTimer* m_timer{nullptr};
        ITestAllocator* m_allocator{nullptr};
        ITestMemoryTracker* m_tracker{nullptr};
        TestState m_state{TestState::Pending};
        TestResult m_result{};
        const char* m_lastNote{nullptr};
    };

    inline TestContext::TestContext(
        ITestLogger& logger,
        ITestTimer& timer,
        ITestAllocator& allocator,
        ITestMemoryTracker& tracker) noexcept :
        m_logger(&logger),
        m_timer(&timer),
        m_allocator(&allocator),
        m_tracker(&tracker) {
        m_result.kind = TestResult::Kind::Success;
        m_state = TestState::Pending;
    }

    using TestFunction = void(*)(ITestContext&) noexcept;

    struct TestResultBuilder {
        TestResult result{};

        [[nodiscard]] static TestResultBuilder Success() noexcept {
            TestResultBuilder builder{};
            builder.result.kind = TestResult::Kind::Success;
            return builder;
        }

        [[nodiscard]] static TestResultBuilder Failure(const char* message = nullptr,
                                                       const char* file = nullptr,
                                                       Int32 line = 0) noexcept {
            TestResultBuilder builder{};
            builder.result.kind = TestResult::Kind::Failure;
            builder.result.message = message;
            builder.result.file = file;
            builder.result.line = line;
            return builder;
        }

        [[nodiscard]] static TestResultBuilder Skipped(const char* message = nullptr) noexcept {
            TestResultBuilder builder{};
            builder.result.kind = TestResult::Kind::Skipped;
            builder.result.message = message;
            return builder;
        }

        [[nodiscard]] static TestResultBuilder Timeout(const char* message = nullptr) noexcept {
            TestResultBuilder builder{};
            builder.result.kind = TestResult::Kind::Timeout;
            builder.result.message = message;
            return builder;
        }

        TestResultBuilder& WithMessage(const char* msg) noexcept {
            result.message = msg;
            return *this;
        }

        TestResultBuilder& AtLocation(const char* file, Int32 line) noexcept {
            result.file = file;
            result.line = line;
            return *this;
        }

        [[nodiscard]] operator TestResult() const noexcept { return result; }

        void BuildAndReport(ITestContext& ctx) const noexcept {
            ctx.SetResult(result);
        }
    };
}
