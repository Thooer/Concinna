module;
export module Log;
import Lang;
import Sys.Memory;
import Prm.Sync;
import Cap.ConcurrentContainers;
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
            m_running.Store(true, Prm::MemoryOrder::Release);
        }
        void Stop() noexcept {
            (void)m_running.Exchange(false, Prm::MemoryOrder::AcqRel);
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
            if (m_target) {
                Record rec{};
                for (;;) {
                    auto r = m_queue.Dequeue(rec);
                    if (!r.IsOk() || !r.Value()) break;
                    m_target->Write(rec);
                }
            }
        }
    private:
        void FlushTLS() noexcept {
            for (USize i = 0; i < g_tlsCount; ++i) {
                (void)m_queue.Enqueue(Move(g_tlsBuf[i]));
            }
            g_tlsCount = 0;
        }
        Cap::MPMCQueue<Record> m_queue{};
        SyncLogger* m_target{};
        Prm::Atomic<bool> m_running{false};
    };
}
