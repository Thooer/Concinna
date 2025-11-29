#define PROF_ZONE(name) ::Profiling::Zone __profiling_zone(name)
module;
export module Profiling;
import Lang;

export namespace Profiling {
    using FnFrameMark = void(*)() noexcept;
    using FnFrameMarkStart = void(*)(const char*) noexcept;
    using FnFrameMarkEnd = void(*)(const char*) noexcept;
    using FnMessage = void(*)(const char*) noexcept;
    using FnPlotD = void(*)(const char*, double) noexcept;
    using FnPlotF = void(*)(const char*, float) noexcept;
    using FnPlotI64 = void(*)(const char*, long long) noexcept;
    using FnPlotU64 = void(*)(const char*, unsigned long long) noexcept;
    using FnPlotI = void(*)(const char*, int) noexcept;
    using FnPlotU = void(*)(const char*, unsigned int) noexcept;
    using FnZoneStart = void(*)(const char*) noexcept;
    using FnZoneEnd = void(*)() noexcept;

    struct Api {
        FnZoneStart ZoneStart;
        FnZoneEnd ZoneEnd;
        FnFrameMark FrameMark;
        FnFrameMarkStart FrameMarkStart;
        FnFrameMarkEnd FrameMarkEnd;
        FnMessage Message;
        FnPlotD PlotD;
        FnPlotF PlotF;
        FnPlotI64 PlotI64;
        FnPlotU64 PlotU64;
        FnPlotI PlotI;
        FnPlotU PlotU;
    };

    void Register(const Api&) noexcept;
    const Api& Get() noexcept;

    struct Zone {
        explicit Zone(const char* name) noexcept { auto& a = Get(); if (a.ZoneStart) a.ZoneStart(name); }
        ~Zone() noexcept { auto& a = Get(); if (a.ZoneEnd) a.ZoneEnd(); }
    };

    inline void FrameMark() noexcept { auto& a = Get(); if (a.FrameMark) a.FrameMark(); }
    inline void FrameMarkStart(const char* name) noexcept { auto& a = Get(); if (a.FrameMarkStart) a.FrameMarkStart(name); }
    inline void FrameMarkEnd(const char* name) noexcept { auto& a = Get(); if (a.FrameMarkEnd) a.FrameMarkEnd(name); }
    inline void Message(const char* text) noexcept { auto& a = Get(); if (a.Message) a.Message(text); }
    inline void Plot(const char* name, double value) noexcept { auto& a = Get(); if (a.PlotD) a.PlotD(name, value); }
    inline void Plot(const char* name, float value) noexcept { auto& a = Get(); if (a.PlotF) a.PlotF(name, value); }
    inline void Plot(const char* name, long long value) noexcept { auto& a = Get(); if (a.PlotI64) a.PlotI64(name, value); }
    inline void Plot(const char* name, unsigned long long value) noexcept { auto& a = Get(); if (a.PlotU64) a.PlotU64(name, value); }
    inline void Plot(const char* name, int value) noexcept { auto& a = Get(); if (a.PlotI) a.PlotI(name, value); }
    inline void Plot(const char* name, unsigned int value) noexcept { auto& a = Get(); if (a.PlotU) a.PlotU(name, value); }
}

namespace {
    Profiling::Api g_api{};
}

export namespace Profiling {
    void Register(const Api& api) noexcept { g_api = api; }
    const Api& Get() noexcept { return g_api; }
}