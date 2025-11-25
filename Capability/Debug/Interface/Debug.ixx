module;
export module Debug;
import Language;
import Platform;

namespace { using CrashHandler = void(*)(const char*) noexcept; CrashHandler g_crashHandler = nullptr; }

export namespace Debug {
    enum class LogLevel : UInt8 { Trace, Debug, Info, Warn, Error, Critical };
    struct Field { const char* key; FormatArg value; };
    struct LogRecord { LogLevel level; const char* category; const char* message; const Field* fields; USize fieldCount; };
    class ILogSink { public: virtual ~ILogSink() = default; virtual void Write(const LogRecord& r) noexcept = 0; };
    class ConsoleSink : public ILogSink {
    public:
        void Write(const LogRecord& r) noexcept override {
            auto h = Platform::File::Stdout();
            WriteStr(h, "[");
            WriteStr(h, LevelName(r.level));
            WriteStr(h, "] ");
            if (r.category) { WriteStr(h, r.category); WriteStr(h, ": "); }
            if (r.message) { WriteStr(h, r.message); }
            if (r.fields && r.fieldCount) {
                WriteStr(h, " ");
                for (USize i = 0; i < r.fieldCount; ++i) {
                    const Field& f = r.fields[i];
                    if (f.key) { WriteStr(h, f.key); WriteStr(h, "="); }
                    WriteArg(h, f.value);
                    if (i + 1 < r.fieldCount) WriteStr(h, " ");
                }
            }
            WriteStr(h, "\n");
        }
    private:
        static void WriteStr(Platform::FileHandle h, const char* s) noexcept {
            if (!s) return;
            USize n = 0; while (s[n] != '\0') ++n;
            (void)Platform::File::Write(h, reinterpret_cast<const Byte*>(s), n);
        }
        static void WriteArg(Platform::FileHandle h, const FormatArg& a) noexcept {
            if (a.kind == FormatArg::Kind::Str) {
                (void)Platform::File::Write(h, reinterpret_cast<const Byte*>(a.u.str.data), a.u.str.size);
                return;
            }
            StaticString<64> tmp;
            FormatArg arr[1]{ a };
            (void)tmp.AppendFormat(StringView("{}"), Span<const FormatArg>(arr, 1));
            const auto v = tmp.View();
            (void)Platform::File::Write(h, reinterpret_cast<const Byte*>(v.data()), v.size());
        }
        static const char* LevelName(LogLevel lv) noexcept {
            switch (lv) {
                case LogLevel::Trace: return "TRACE";
                case LogLevel::Debug: return "DEBUG";
                case LogLevel::Info: return "INFO";
                case LogLevel::Warn: return "WARN";
                case LogLevel::Error: return "ERROR";
                case LogLevel::Critical: return "CRIT";
                default: return "?";
            }
        }
    };
    class RotatingFileSink : public ILogSink {
    public:
        RotatingFileSink(const char* basePath, UInt64 maxBytes) noexcept : m_base(basePath), m_max(maxBytes) { OpenNew(); }
        ~RotatingFileSink() noexcept { if (m_file.Get()) (void)Platform::File::Close(m_file); }
        void Write(const LogRecord& r) noexcept override {
            CheckRotate();
            WriteStr("[");
            WriteStr(LevelName(r.level));
            WriteStr("] ");
            if (r.category) { WriteStr(r.category); WriteStr(": "); }
            if (r.message) { WriteStr(r.message); }
            if (r.fields && r.fieldCount) {
                WriteStr(" ");
                for (USize i = 0; i < r.fieldCount; ++i) {
                    const Field& f = r.fields[i];
                    if (f.key) { WriteStr(f.key); WriteStr("="); }
                    WriteArg(f.value);
                    if (i + 1 < r.fieldCount) WriteStr(" ");
                }
            }
            WriteStr("\n");
        }
    private:
        void OpenNew() noexcept {
            char name[256];
            USize n = 0;
            const char* b = m_base ? m_base : "log";
            while (b[n] != '\0' && n < 200) { name[n] = b[n]; ++n; }
            name[n++] = '.';
            name[n++] = static_cast<char>('0' + (m_index % 10));
            name[n++] = static_cast<char>('0' + ((m_index / 10) % 10));
            name[n++] = static_cast<char>('0' + ((m_index / 100) % 10));
            name[n++] = '.';
            name[n++] = 'l'; name[n++] = 'o'; name[n++] = 'g';
            name[n] = '\0';
            auto e = Platform::File::Open(StringView(name), Platform::FileOpenMode::Write, Platform::FileShareMode::Read);
            if (e.IsOk()) { m_file = e.Value(); m_size = 0; }
        }
        void CheckRotate() noexcept {
            if (m_max == 0) return;
            if (m_size >= m_max) { if (m_file.Get()) (void)Platform::File::Close(m_file); ++m_index; OpenNew(); }
        }
        void WriteStr(const char* s) noexcept {
            if (!s) return;
            USize n = 0; while (s[n] != '\0') ++n;
            m_size += n + 1;
            (void)Platform::File::Write(m_file, reinterpret_cast<const Byte*>(s), n);
        }
        static const char* LevelName(LogLevel lv) noexcept {
            switch (lv) {
                case LogLevel::Trace: return "TRACE";
                case LogLevel::Debug: return "DEBUG";
                case LogLevel::Info: return "INFO";
                case LogLevel::Warn: return "WARN";
                case LogLevel::Error: return "ERROR";
                case LogLevel::Critical: return "CRIT";
                default: return "?";
            }
        }
        void WriteArg(const FormatArg& a) noexcept {
            if (a.kind == FormatArg::Kind::Str) {
                (void)Platform::File::Write(m_file, reinterpret_cast<const Byte*>(a.u.str.data), a.u.str.size);
                return;
            }
            StaticString<64> tmp;
            FormatArg arr[1]{ a };
            (void)tmp.AppendFormat(StringView("{}"), Span<const FormatArg>(arr, 1));
            const auto v = tmp.View();
            (void)Platform::File::Write(m_file, reinterpret_cast<const Byte*>(v.data()), v.size());
        }
        const char* m_base{};
        Platform::FileHandle m_file{};
        UInt64 m_size{};
        UInt64 m_max{};
        UInt32 m_index{};
    };
    class Logger {
    public:
        void AddSink(ILogSink* s) noexcept { if (m_count < kMax) m_sinks[m_count++] = s; }
        void Log(LogLevel lv, const char* cat, const char* msg) noexcept { LogRecord r{lv, cat, msg, nullptr, 0}; Write(r); }
        void LogWithFields(LogLevel lv, const char* cat, const char* msg, const Field* fields, USize n) noexcept { LogRecord r{lv, cat, msg, fields, n}; Write(r); }
        void Write(const LogRecord& r) noexcept { for (USize i = 0; i < m_count; ++i) { if (m_sinks[i]) m_sinks[i]->Write(r); } }
        static void Panic(const char* msg) noexcept {
            if (g_crashHandler) { g_crashHandler(msg); }
            auto h = Platform::File::Stdout();
            WriteStr(h, "PANIC: ");
            WriteStr(h, msg);
            WriteStr(h, "\n");
            DebugBreak();
        }
        static void PanicFormat(StringView fmt, Span<const FormatArg> args) noexcept {
            StaticString<1024> buf;
            buf.Append(StringView("PANIC: "));
            (void)buf.AppendFormat(fmt, args);
            auto h = Platform::File::Stdout();
            const auto v = buf.View();
            USize n = v.size();
            (void)Platform::File::Write(h, reinterpret_cast<const Byte*>(v.data()), n);
            WriteStr(h, "\n");
            DebugBreak();
        }
        static void Assert(bool cond, const char* msg) noexcept { if (!cond) Panic(msg); }
        static void SetCrashHandler(void(*fn)(const char*) noexcept) noexcept { g_crashHandler = fn; }
    private:
        static void WriteStr(Platform::FileHandle h, const char* s) noexcept {
            if (!s) return;
            USize n = 0; while (s[n] != '\0') ++n;
            (void)Platform::File::Write(h, reinterpret_cast<const Byte*>(s), n);
        }
        static constexpr USize kMax = 8;
        ILogSink* m_sinks[kMax]{};
        USize m_count{0};
    };
}