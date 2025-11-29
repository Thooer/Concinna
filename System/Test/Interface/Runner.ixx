module;

export module Sys.Job.Test:Runner;

import <exception>;
import <new>;

import Lang;
import Cap.Memory;
import Cap.Concurrency;
import Prm.Sync;
import Sys;
//

import :Core;
import :Context;
import :Registration;

export namespace Sys {

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
        Cap::Atomic<UInt64>* passed{nullptr};
        Cap::Atomic<UInt64>* failed{nullptr};
        Cap::Atomic<UInt64>* skipped{nullptr};
        Cap::Atomic<UInt64>* timed{nullptr};
        Cap::Atomic<UInt64>* totalMicros{nullptr};
        Cap::Counter* counter{nullptr};
        };

        inline void ExecuteTest(JobPayload& payload) noexcept {
            Cap::FrameAllocatorResource arena(payload.config.frameArenaSize);
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
            payload.totalMicros->FetchAdd(micros, Prm::MemoryOrder::AcqRel);

            const auto result = context.GetResult();
            if (result.IsSuccess()) { payload.passed->FetchAdd(1, Prm::MemoryOrder::AcqRel); }
            else if (result.IsSkipped()) { payload.skipped->FetchAdd(1, Prm::MemoryOrder::AcqRel); }
            else if (result.IsTimeout()) { payload.timed->FetchAdd(1, Prm::MemoryOrder::AcqRel); }
            else { payload.failed->FetchAdd(1, Prm::MemoryOrder::AcqRel); }
        }

        inline void JobEntry(void* arg) noexcept {
            auto* payload = static_cast<JobPayload*>(arg);
            ExecuteTest(*payload);
            if (payload->counter) { payload->counter->SignalComplete(); }
            delete payload;
        }
    }

    inline RunSummary RunRegistered(const RunConfig& config = {}) noexcept {
        RunSummary summary{};
        auto& registry = Registry();
        auto total = registry.Count();
        summary.statistics.totalTests = total;

        auto workers = config.workerCount == 0 ? 1u : config.workerCount;

        Cap::Atomic<UInt64> passed{0};
        Cap::Atomic<UInt64> failed{0};
        Cap::Atomic<UInt64> skipped{0};
        Cap::Atomic<UInt64> timed{0};
        Cap::Atomic<UInt64> totalMicros{0};

        (void)Sys::JobStart(workers);
        Cap::Counter counter{};
        counter.Reset(static_cast<UInt32>(total));

        for (USize i = 0; i < total; ++i) {
            auto* payload = new(std::nothrow) Detail::JobPayload{};
            if (!payload) {
                (void)failed.FetchAdd(1, Prm::MemoryOrder::AcqRel);
                continue;
            }
            payload->entry = &registry.Get(i);
            payload->config = config;
            payload->passed = &passed;
            payload->failed = &failed;
            payload->skipped = &skipped;
            payload->timed = &timed;
            payload->totalMicros = &totalMicros;
            payload->counter = &counter;

            Cap::Job job{};
            job.invoke = &Detail::JobEntry;
            job.arg = payload;
            (void)Sys::RunWithCounter(job, counter);
        }
        Sys::WaitForCounter(counter);
        Sys::JobStop();

        summary.statistics.passedTests = passed.Load(Prm::MemoryOrder::Acquire);
        summary.statistics.failedTests = failed.Load(Prm::MemoryOrder::Acquire);
        summary.statistics.skippedTests = skipped.Load(Prm::MemoryOrder::Acquire);
        summary.statistics.timedOutTests = timed.Load(Prm::MemoryOrder::Acquire);
        summary.statistics.totalTimeMs = static_cast<Float64>(totalMicros.Load(Prm::MemoryOrder::Acquire)) / 1000.0;
        summary.statistics.averageTimeMs = summary.statistics.totalTests == 0
            ? 0.0
            : summary.statistics.totalTimeMs / static_cast<Float64>(summary.statistics.totalTests);

        summary.exitCode = (summary.statistics.failedTests == 0 && summary.statistics.timedOutTests == 0) ? 0 : 1;
        return summary;
    }
}
