module;

export module Test:Runner;

import <exception>;
import <new>;

import Language;
import Memory;
import Concurrency;

import :Core;
import :Context;
import :Registration;

export namespace Test {

    struct RunConfig {
        UInt32 workerCount{4};
        USize frameArenaSize{256u << 10u};
        LogLevel logLevel{LogLevel::Info};
        bool failOnLeak{true};
    };

    struct RunSummary {
        TestStatistics statistics{};
        Int32 exitCode{0};
    };

    namespace Detail {
        struct JobPayload {
            const TestEntry* entry{nullptr};
            RunConfig config{};
            Atomic<UInt64>* passed{nullptr};
            Atomic<UInt64>* failed{nullptr};
            Atomic<UInt64>* skipped{nullptr};
            Atomic<UInt64>* timed{nullptr};
            Atomic<UInt64>* totalMicros{nullptr};
        };

        inline void ExecuteTest(JobPayload& payload) noexcept {
            Memory::FrameAllocatorResource arena(payload.config.frameArenaSize);
            FrameAllocatorAdapter allocator(arena);
            FrameMemoryTracker tracker;
            HighResolutionTestTimer timer;
            ConsoleTestLogger logger(payload.config.logLevel);
            TestContext context(logger, timer, allocator, tracker);
            context.SetState(TestState::Running);
            timer.Start();

            auto invoke = [&]() noexcept {
                if (payload.entry && payload.entry->fn) {
                    payload.entry->fn(context);
                }
            };

            try {
                invoke();
            } catch (const std::exception& ex) {
                context.Logger().Error(ex.what());
                TestResultBuilder::Failure("Unhandled std::exception", payload.entry ? payload.entry->name : nullptr, 0)
                    .WithMessage(ex.what())
                    .BuildAndReport(context);
            } catch (...) {
                context.Logger().Error("Unhandled exception in test");
                TestResultBuilder::Failure("Unhandled exception", payload.entry ? payload.entry->name : nullptr, 0)
                    .BuildAndReport(context);
            }

            timer.Stop();

            if (payload.config.failOnLeak && arena.Offset() != 0) {
                TestResultBuilder::Failure("Memory leak detected after test")
                    .AtLocation(payload.entry ? payload.entry->name : nullptr, 0)
                    .BuildAndReport(context);
            }

            context.SetState(TestState::Completed);

            auto micros = static_cast<UInt64>(timer.GetElapsedTimeMs() * 1000.0);
            payload.totalMicros->fetch_add(micros);

            const auto result = context.GetResult();
            if (result.IsSuccess()) {
                payload.passed->fetch_add(1);
            } else if (result.IsSkipped()) {
                payload.skipped->fetch_add(1);
            } else if (result.IsTimeout()) {
                payload.timed->fetch_add(1);
            } else {
                payload.failed->fetch_add(1);
            }
        }

        inline void JobEntry(void* arg) noexcept {
            auto* payload = static_cast<JobPayload*>(arg);
            ExecuteTest(*payload);
            delete payload;
        }
    }

    inline RunSummary RunRegistered(const RunConfig& config = {}) noexcept {
        RunSummary summary{};
        auto& registry = Registry();
        auto total = registry.Count();
        summary.statistics.totalTests = total;

        auto workers = config.workerCount == 0 ? 1u : config.workerCount;
        auto& scheduler = Concurrency::Scheduler::Instance();
        if (!scheduler.Start(workers).Ok()) {
            summary.exitCode = 1;
            return summary;
        }

        Concurrency::Counter counter{};
        counter.Reset(static_cast<UInt32>(total));

        Atomic<UInt64> passed{0};
        Atomic<UInt64> failed{0};
        Atomic<UInt64> skipped{0};
        Atomic<UInt64> timed{0};
        Atomic<UInt64> totalMicros{0};

        for (USize i = 0; i < total; ++i) {
            auto* payload = new(std::nothrow) Detail::JobPayload{};
            if (!payload) {
                failed.fetch_add(1);
                continue;
            }
            payload->entry = &registry.Get(i);
            payload->config = config;
            payload->passed = &passed;
            payload->failed = &failed;
            payload->skipped = &skipped;
            payload->timed = &timed;
            payload->totalMicros = &totalMicros;

            Concurrency::Job job{};
            job.invoke = &Detail::JobEntry;
            job.arg = payload;
            if (!Concurrency::RunWithCounter(job, counter).Ok()) {
                delete payload;
                failed.fetch_add(1);
            }
        }

        Concurrency::WaitForCounter(counter);
        (void)scheduler.Stop();

        summary.statistics.passedTests = passed.load();
        summary.statistics.failedTests = failed.load();
        summary.statistics.skippedTests = skipped.load();
        summary.statistics.timedOutTests = timed.load();
        summary.statistics.totalTimeMs = static_cast<Float64>(totalMicros.load()) / 1000.0;
        summary.statistics.averageTimeMs = summary.statistics.totalTests == 0
            ? 0.0
            : summary.statistics.totalTimeMs / static_cast<Float64>(summary.statistics.totalTests);

        summary.exitCode = (summary.statistics.failedTests == 0 && summary.statistics.timedOutTests == 0) ? 0 : 1;
        return summary;
    }
}
