module;
export module Log;
import Language;
import Platform;
import Concurrency;
import Debug;

namespace {
    thread_local Debug::LogRecord g_tlsBuf[64];
    thread_local USize g_tlsCount = 0;
}

export namespace Log {
    using Record = Debug::LogRecord;
    using Level = Debug::LogLevel;
    using SyncLogger = Debug::Logger;

    class AsyncLogger {
    public:
        AsyncLogger() noexcept = default;
        ~AsyncLogger() noexcept { Stop(); }
        void Start(SyncLogger& target) noexcept {
            m_target = &target;
            m_running.Store(true, MemoryOrder::Release);
            auto r = Platform::ThreadCreate(&ThreadProc, this);
            if (r.IsOk()) m_thread = r.Value();
        }
        void Stop() noexcept {
            bool was = m_running.Exchange(false, MemoryOrder::AcqRel);
            if (was && m_thread.Get()) { (void)Platform::ThreadJoin(m_thread); m_thread = Platform::ThreadHandle{}; }
        }
        void Log(Level lv, const char* cat, const char* msg) noexcept {
            g_tlsBuf[g_tlsCount++] = Record{lv, cat, msg};
            if (g_tlsCount >= 64) { FlushTLS(); }
        }
        void LogWithFields(Level lv, const char* cat, const char* msg, const Debug::Field* fields, USize n) noexcept {
            g_tlsBuf[g_tlsCount++] = Record{lv, cat, msg, fields, n};
            if (g_tlsCount >= 64) { FlushTLS(); }
        }
        void LogBatch(const Record* recs, USize n) noexcept {
            for (USize i = 0; i < n; ++i) {
                g_tlsBuf[g_tlsCount++] = recs[i];
                if (g_tlsCount >= 64) { FlushTLS(); }
            }
        }
        void Flush() noexcept {
            if (g_tlsCount > 0) { FlushTLS(); }
        }
    private:
        void FlushTLS() noexcept {
            for (USize i = 0; i < g_tlsCount; ++i) {
                (void)m_queue.Enqueue(Move(g_tlsBuf[i]));
            }
            g_tlsCount = 0;
        }
        static void ThreadProc(void* user) noexcept { static_cast<AsyncLogger*>(user)->Run(); }
        void Run() noexcept {
            for (;;) {
                if (!m_running.Load(MemoryOrder::Acquire)) break;
                if (g_tlsCount > 0) { FlushTLS(); }
                Record rec{};
                auto r = m_queue.Dequeue(rec);
                if (r.IsOk() && r.Value()) {
                    if (m_target) m_target->Write(rec);
                } else {
                    Platform::ThreadSleepMs(1);
                }
            }
        }
        Concurrency::MPMCQueue<Record> m_queue{};
        Platform::ThreadHandle m_thread{};
        SyncLogger* m_target{};
        Atomic<bool> m_running{false};
    };
}