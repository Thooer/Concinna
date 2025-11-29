module;
export module Profiling:NativeBackend;
import Profiling;
import Lang;
import Cap.Stream;
import Prm.Time;

namespace {
    struct Event { UInt64 ts; UInt8 type; UInt64 sid; double valueD; };
    alignas(64) thread_local Event g_ring[1024];
    thread_local UInt32 g_head = 0;
    Cap::FileWriter g_out{Prm::FileHandle{nullptr}};
    UInt64 HashStr(const char* s) noexcept {
        return Fnv1aString(s);
    }
    void EnsureOpen() noexcept {
        if (g_out.Handle() == nullptr) {
            StringView sv("profiling_native.bin");
            auto e = Cap::FileWriter::Open(Span<const Char8, DynamicExtent>{sv.data(), sv.size()}, true);
            if (e.IsOk()) g_out = e.Value();
        }
    }
    void FlushChunk() noexcept {
        EnsureOpen();
        if (g_out.Handle() != nullptr) {
            (void)g_out.WriteBytes(Span<const Byte, DynamicExtent>{reinterpret_cast<const Byte*>(g_ring), static_cast<USize>(sizeof(g_ring))});
        }
    }
    void FlushPartial() noexcept {
        EnsureOpen();
        if (g_out.Handle() != nullptr) {
            UInt32 n = g_head % 1024;
            if (n) {
                (void)g_out.WriteBytes(Span<const Byte, DynamicExtent>{reinterpret_cast<const Byte*>(g_ring), static_cast<USize>(static_cast<USize>(n) * static_cast<USize>(sizeof(Event)))});
            }
        }
    }
    void Push(const Event& e) noexcept { g_ring[g_head++ % 1024] = e; if ((g_head % 1024) == 0) FlushChunk(); }
    UInt64 Now() noexcept { return static_cast<UInt64>(Prm::Now()); }
    void ZoneStart(const char* name) noexcept { Event e{Now(), 1, HashStr(name), 0.0}; Push(e); }
    void ZoneEnd() noexcept { Event e{Now(), 2, 0, 0.0}; Push(e); }
    void FrameMark() noexcept { Event e{Now(), 3, 0, 0.0}; Push(e); }
    void FrameMarkStart(const char* name) noexcept { Event e{Now(), 4, HashStr(name), 0.0}; Push(e); }
    void FrameMarkEnd(const char* name) noexcept { Event e{Now(), 5, HashStr(name), 0.0}; Push(e); }
    void Message(const char* text) noexcept { Event e{Now(), 6, HashStr(text), 0.0}; Push(e); }
    void PlotD(const char* name, double value) noexcept { Event e{Now(), 7, HashStr(name), value}; Push(e); }
    void PlotF(const char* name, float value) noexcept { PlotD(name, static_cast<double>(value)); }
    void PlotI64(const char* name, long long value) noexcept { PlotD(name, static_cast<double>(value)); }
    void PlotU64(const char* name, unsigned long long value) noexcept { PlotD(name, static_cast<double>(value)); }
    void PlotI(const char* name, int value) noexcept { PlotD(name, static_cast<double>(value)); }
    void PlotU(const char* name, unsigned int value) noexcept { PlotD(name, static_cast<double>(value)); }
    struct ThreadFlushGuard { ~ThreadFlushGuard() noexcept { FlushPartial(); } };
    thread_local ThreadFlushGuard g_threadFlushGuard{};
}

export namespace Profiling::Backend::Native {
    void Init() noexcept {
        Profiling::Api api{};
        api.ZoneStart = &ZoneStart;
        api.ZoneEnd = &ZoneEnd;
        api.FrameMark = &FrameMark;
        api.FrameMarkStart = &FrameMarkStart;
        api.FrameMarkEnd = &FrameMarkEnd;
        api.Message = &Message;
        api.PlotD = &PlotD;
        api.PlotF = &PlotF;
        api.PlotI64 = &PlotI64;
        api.PlotU64 = &PlotU64;
        api.PlotI = &PlotI;
        api.PlotU = &PlotU;
        Profiling::Register(api);
    }
}
