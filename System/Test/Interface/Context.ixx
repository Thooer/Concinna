module;

import <chrono>;
import <cstdarg>;
import <cstdio>;

export module Test:Context;

import Language;
import Memory;

import :Core;

export namespace Test {

    enum class LogLevel : UInt8 {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    class ITestLogger {
    public:
        virtual ~ITestLogger() = default;
        virtual void Log(LogLevel level, const char* message) noexcept = 0;
        virtual void LogFormat(LogLevel level, const char* format, ...) noexcept = 0;
        virtual void LogFormatV(LogLevel level, const char* format, va_list args) noexcept = 0;
        virtual void Trace(const char* message) noexcept = 0;
        virtual void Debug(const char* message) noexcept = 0;
        virtual void Info(const char* message) noexcept = 0;
        virtual void Warning(const char* message) noexcept = 0;
        virtual void Error(const char* message) noexcept = 0;
        virtual void Critical(const char* message) noexcept = 0;
        virtual void SetLevel(LogLevel level) noexcept = 0;
        [[nodiscard]] virtual LogLevel GetLevel() const noexcept = 0;
    };

    class ConsoleTestLogger final : public ITestLogger {
    public:
        explicit ConsoleTestLogger(LogLevel level = LogLevel::Info) noexcept : m_level(level) {}

        void Log(LogLevel level, const char* message) noexcept override {
            if (level < m_level) { return; }
            std::fprintf(stdout, "[%s] %s\n", LevelName(level), message ? message : "");
        }

        void LogFormat(LogLevel level, const char* format, ...) noexcept override {
            va_list args;
            va_start(args, format);
            LogFormatV(level, format, args);
            va_end(args);
        }

        void LogFormatV(LogLevel level, const char* format, va_list args) noexcept override {
            if (level < m_level) { return; }
            char buffer[1024];
            std::vsnprintf(buffer, sizeof(buffer), format ? format : "", args);
            Log(level, buffer);
        }

        void Trace(const char* message) noexcept override { Log(LogLevel::Trace, message); }
        void Debug(const char* message) noexcept override { Log(LogLevel::Debug, message); }
        void Info(const char* message) noexcept override { Log(LogLevel::Info, message); }
        void Warning(const char* message) noexcept override { Log(LogLevel::Warning, message); }
        void Error(const char* message) noexcept override { Log(LogLevel::Error, message); }
        void Critical(const char* message) noexcept override { Log(LogLevel::Critical, message); }

        void SetLevel(LogLevel level) noexcept override { m_level = level; }
        [[nodiscard]] LogLevel GetLevel() const noexcept override { return m_level; }

    private:
        LogLevel m_level;

        [[nodiscard]] const char* LevelName(LogLevel level) const noexcept {
            switch (level) {
                case LogLevel::Trace: return "TRACE";
                case LogLevel::Debug: return "DEBUG";
                case LogLevel::Info: return "INFO";
                case LogLevel::Warning: return "WARN";
                case LogLevel::Error: return "ERROR";
                case LogLevel::Critical: return "CRIT";
                default: return "LOG";
            }
        }
    };

    class ITestTimer {
    public:
        virtual ~ITestTimer() = default;
        [[nodiscard]] virtual Float64 GetElapsedTimeMs() const noexcept = 0;
        [[nodiscard]] virtual Float64 GetDeltaTime() const noexcept = 0;
        [[nodiscard]] virtual UInt64 GetTickCount() const noexcept = 0;
        virtual void Start() noexcept = 0;
        virtual void Stop() noexcept = 0;
        virtual void Reset() noexcept = 0;
        virtual void BeginFrame() noexcept = 0;
        virtual void EndFrame() noexcept = 0;
        [[nodiscard]] virtual Float64 GetFrameTime() const noexcept = 0;
        [[nodiscard]] virtual Float64 GetResolution() const noexcept = 0;
        [[nodiscard]] virtual bool IsHighResolution() const noexcept = 0;
    };

    class HighResolutionTestTimer final : public ITestTimer {
    public:
        HighResolutionTestTimer() = default;

        [[nodiscard]] Float64 GetElapsedTimeMs() const noexcept override {
            return static_cast<Float64>(m_endTick - m_startTick) / 1000.0;
        }

        [[nodiscard]] Float64 GetDeltaTime() const noexcept override { return m_frameTime; }
        [[nodiscard]] UInt64 GetTickCount() const noexcept override { return m_endTick; }

        void Start() noexcept override {
            auto now = std::chrono::steady_clock::now().time_since_epoch();
            m_startTick = static_cast<UInt64>(std::chrono::duration_cast<std::chrono::microseconds>(now).count());
            m_endTick = m_startTick;
        }

        void Stop() noexcept override {
            auto now = std::chrono::steady_clock::now().time_since_epoch();
            m_endTick = static_cast<UInt64>(std::chrono::duration_cast<std::chrono::microseconds>(now).count());
        }

        void Reset() noexcept override {
            m_startTick = 0;
            m_endTick = 0;
            m_frameStart = 0;
            m_frameEnd = 0;
            m_frameTime = 0.0;
        }

        void BeginFrame() noexcept override {
            auto now = std::chrono::steady_clock::now().time_since_epoch();
            m_frameStart = static_cast<UInt64>(std::chrono::duration_cast<std::chrono::microseconds>(now).count());
        }

        void EndFrame() noexcept override {
            auto now = std::chrono::steady_clock::now().time_since_epoch();
            m_frameEnd = static_cast<UInt64>(std::chrono::duration_cast<std::chrono::microseconds>(now).count());
            auto delta = (m_frameEnd >= m_frameStart) ? (m_frameEnd - m_frameStart) : 0;
            m_frameTime = static_cast<Float64>(delta) / 1000.0;
        }

        [[nodiscard]] Float64 GetFrameTime() const noexcept override { return m_frameTime; }
        [[nodiscard]] Float64 GetResolution() const noexcept override { return 0.001; }
        [[nodiscard]] bool IsHighResolution() const noexcept override { return true; }

    private:
        UInt64 m_startTick{0};
        UInt64 m_endTick{0};
        UInt64 m_frameStart{0};
        UInt64 m_frameEnd{0};
        Float64 m_frameTime{0.0};
    };

    class ITestAllocator {
    public:
        virtual ~ITestAllocator() = default;
        [[nodiscard]] virtual void* Allocate(UInt64 size, UInt32 alignment = 0) noexcept = 0;
        virtual void Deallocate(void* ptr) noexcept = 0;
        [[nodiscard]] virtual void* Reallocate(void* ptr, UInt64 newSize) noexcept = 0;
        [[nodiscard]] virtual UInt64 GetTotalAllocated() const noexcept = 0;
        [[nodiscard]] virtual UInt64 GetCurrentUsed() const noexcept = 0;
        [[nodiscard]] virtual UInt64 GetPeakUsed() const noexcept = 0;
        [[nodiscard]] virtual UInt32 GetAllocationCount() const noexcept = 0;
        [[nodiscard]] virtual UInt32 GetAlignment() const noexcept = 0;
        virtual void SetAlignment(UInt32 alignment) noexcept = 0;
        virtual void Reset() noexcept = 0;
    };

    class FrameAllocatorAdapter final : public ITestAllocator {
    public:
        explicit FrameAllocatorAdapter(Memory::FrameAllocatorResource& resource) noexcept : m_resource(&resource) {}

        [[nodiscard]] void* Allocate(UInt64 size, UInt32 alignment = 0) noexcept override {
            auto align = alignment == 0 ? static_cast<USize>(16) : static_cast<USize>(alignment);
            auto block = m_resource->Allocate(static_cast<USize>(size), align);
            if (!block.Ok()) { return nullptr; }
            auto used = m_currentUsed.fetch_add(block.Value().size) + block.Value().size;
            if (used > m_peakUsed.load()) { m_peakUsed.store(used); }
            m_allocationCount.fetch_add(1);
            return block.Value().ptr;
        }

        void Deallocate(void* /*ptr*/) noexcept override {}

        [[nodiscard]] void* Reallocate(void* /*ptr*/, UInt64 /*newSize*/) noexcept override { return nullptr; }

        [[nodiscard]] UInt64 GetTotalAllocated() const noexcept override { return m_resource->m_capacity; }
        [[nodiscard]] UInt64 GetCurrentUsed() const noexcept override { return m_resource->Offset(); }
        [[nodiscard]] UInt64 GetPeakUsed() const noexcept override { return m_peakUsed.load(); }
        [[nodiscard]] UInt32 GetAllocationCount() const noexcept override { return static_cast<UInt32>(m_allocationCount.load()); }
        [[nodiscard]] UInt32 GetAlignment() const noexcept override { return m_alignment; }
        void SetAlignment(UInt32 alignment) noexcept override { m_alignment = alignment == 0 ? 16u : alignment; }
        void Reset() noexcept override {
            m_resource->Reset();
            m_currentUsed.store(0);
            m_peakUsed.store(0);
            m_allocationCount.store(0);
        }

    private:
        Memory::FrameAllocatorResource* m_resource;
        Atomic<UInt64> m_currentUsed{0};
        Atomic<UInt64> m_peakUsed{0};
        Atomic<UInt64> m_allocationCount{0};
        UInt32 m_alignment{16};
    };

    struct AllocationInfo {
        void* ptr{nullptr};
        UInt64 size{0};
        UInt32 alignment{0};
        const char* file{nullptr};
        Int32 line{0};
        const char* tag{nullptr};
    };

    class ITestMemoryTracker {
    public:
        virtual ~ITestMemoryTracker() = default;
        virtual void StartTracking() noexcept = 0;
        virtual void StopTracking() noexcept = 0;
        [[nodiscard]] virtual bool IsTracking() const noexcept = 0;
        virtual void RecordAllocation(void* ptr, UInt64 size, UInt32 alignment = 0,
                                     const char* file = nullptr, Int32 line = 0,
                                     const char* tag = nullptr) noexcept = 0;
        virtual void RecordDeallocation(void* ptr) noexcept = 0;
        [[nodiscard]] virtual UInt64 GetLeakedCount() const noexcept = 0;
        [[nodiscard]] virtual UInt64 GetLeakedSize() const noexcept = 0;
        [[nodiscard]] virtual bool HasLeaks() const noexcept = 0;
        virtual void GenerateLeakReport() noexcept = 0;
        virtual void ClearLeaks() noexcept = 0;
        [[nodiscard]] virtual UInt64 GetTotalAllocations() const noexcept = 0;
        [[nodiscard]] virtual UInt64 GetTotalDeallocations() const noexcept = 0;
        [[nodiscard]] virtual Float64 GetAverageAllocationSize() const noexcept = 0;
        [[nodiscard]] virtual AllocationInfo GetAllocationInfo(void* ptr) const noexcept = 0;
        [[nodiscard]] virtual bool IsValidAllocation(void* ptr) const noexcept = 0;
    };

    class FrameMemoryTracker final : public ITestMemoryTracker {
    public:
        FrameMemoryTracker() = default;

        void StartTracking() noexcept override {
            m_tracking = true;
            ClearLeaks();
        }

        void StopTracking() noexcept override { m_tracking = false; }
        [[nodiscard]] bool IsTracking() const noexcept override { return m_tracking; }

        void RecordAllocation(void* ptr, UInt64 size, UInt32 alignment = 0,
                             const char* file = nullptr, Int32 line = 0,
                             const char* tag = nullptr) noexcept override {
            if (!m_tracking || ptr == nullptr || m_recordCount >= kMaxRecords) { return; }
            m_records[m_recordCount++] = AllocationInfo{ptr, size, alignment, file, line, tag};
            m_totalAllocations++;
            m_totalBytes += size;
        }

        void RecordDeallocation(void* ptr) noexcept override {
            if (!m_tracking || ptr == nullptr) { return; }
            for (USize i = 0; i < m_recordCount; ++i) {
                if (m_records[i].ptr == ptr) {
                    m_totalDeallocations++;
                    m_totalBytes -= m_records[i].size;
                    m_records[i] = m_records[m_recordCount - 1];
                    --m_recordCount;
                    return;
                }
            }
        }

        [[nodiscard]] UInt64 GetLeakedCount() const noexcept override { return m_leakedCount; }
        [[nodiscard]] UInt64 GetLeakedSize() const noexcept override { return m_leakedSize; }
        [[nodiscard]] bool HasLeaks() const noexcept override { return m_leakedCount > 0; }

        void GenerateLeakReport() noexcept override {
            m_leakedCount = m_recordCount;
            UInt64 total{0};
            for (USize i = 0; i < m_recordCount; ++i) {
                total += m_records[i].size;
            }
            m_leakedSize = total;
        }

        void ClearLeaks() noexcept override {
            m_recordCount = 0;
            m_leakedCount = 0;
            m_leakedSize = 0;
            m_totalAllocations = 0;
            m_totalDeallocations = 0;
            m_totalBytes = 0;
        }

        [[nodiscard]] UInt64 GetTotalAllocations() const noexcept override { return m_totalAllocations; }
        [[nodiscard]] UInt64 GetTotalDeallocations() const noexcept override { return m_totalDeallocations; }
        [[nodiscard]] Float64 GetAverageAllocationSize() const noexcept override {
            if (m_totalAllocations == 0) { return 0.0; }
            return static_cast<Float64>(m_totalBytes) / static_cast<Float64>(m_totalAllocations);
        }

        [[nodiscard]] AllocationInfo GetAllocationInfo(void* ptr) const noexcept override {
            for (USize i = 0; i < m_recordCount; ++i) {
                if (m_records[i].ptr == ptr) { return m_records[i]; }
            }
            return AllocationInfo{};
        }

        [[nodiscard]] bool IsValidAllocation(void* ptr) const noexcept override {
            for (USize i = 0; i < m_recordCount; ++i) {
                if (m_records[i].ptr == ptr) { return true; }
            }
            return false;
        }

    private:
        static constexpr USize kMaxRecords = 1024;
        AllocationInfo m_records[kMaxRecords];
        USize m_recordCount{0};
        bool m_tracking{false};
        UInt64 m_totalAllocations{0};
        UInt64 m_totalDeallocations{0};
        UInt64 m_totalBytes{0};
        UInt64 m_leakedCount{0};
        UInt64 m_leakedSize{0};
    };

    struct DefaultContextFactory {
        [[nodiscard]] static TestContext Create(LogLevel level = LogLevel::Info,
                                                USize frameSize = 64u << 10) noexcept {
            static ConsoleTestLogger s_logger(level);
            static HighResolutionTestTimer s_timer;
            static Memory::FrameAllocatorResource s_frame(frameSize);
            static FrameAllocatorAdapter s_allocator(s_frame);
            static FrameMemoryTracker s_tracker;
            return TestContext(s_logger, s_timer, s_allocator, s_tracker);
        }
    };

    namespace ContextUtils {
        [[nodiscard]] inline ConsoleTestLogger CreateDefaultLogger(LogLevel level = LogLevel::Info) noexcept {
            return ConsoleTestLogger(level);
        }

        [[nodiscard]] inline HighResolutionTestTimer CreateDefaultTimer() noexcept {
            return HighResolutionTestTimer();
        }

        [[nodiscard]] inline FrameAllocatorAdapter CreateFrameAllocator(Memory::FrameAllocatorResource& resource) noexcept {
            return FrameAllocatorAdapter(resource);
        }

        [[nodiscard]] inline FrameMemoryTracker CreateMemoryTracker() noexcept {
            return FrameMemoryTracker();
        }

        inline void Logf(ITestLogger& logger, LogLevel level, const char* format, ...) noexcept {
            va_list args;
            va_start(args, format);
            logger.LogFormatV(level, format, args);
            va_end(args);
        }
    }
}
