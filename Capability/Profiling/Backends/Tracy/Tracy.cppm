module;
export module Profiling:TracyBackend;
import Profiling;
import Tools.Tracy;

namespace {
    alignas(sizeof(Tools::Tracy::Zone)) thread_local unsigned char g_zoneStorage[sizeof(Tools::Tracy::Zone)];
    thread_local bool g_zoneActive = false;
    void ZoneStart(const char* name) noexcept {
        if (!g_zoneActive) {
            new (g_zoneStorage) Tools::Tracy::Zone{name};
            g_zoneActive = true;
        }
    }
    void ZoneEnd() noexcept {
        if (g_zoneActive) {
            reinterpret_cast<Tools::Tracy::Zone*>(g_zoneStorage)->~Zone();
            g_zoneActive = false;
        }
    }
    void FrameMark() noexcept { Tools::Tracy::FrameMark(); }
    void FrameMarkStart(const char* name) noexcept { Tools::Tracy::FrameMarkStart(name); }
    void FrameMarkEnd(const char* name) noexcept { Tools::Tracy::FrameMarkEnd(name); }
    void Message(const char* text) noexcept { Tools::Tracy::Message(text); }
    void PlotD(const char* name, double value) noexcept { Tools::Tracy::Plot(name, value); }
    void PlotF(const char* name, float value) noexcept { Tools::Tracy::Plot(name, value); }
    void PlotI64(const char* name, long long value) noexcept { Tools::Tracy::Plot(name, value); }
    void PlotU64(const char* name, unsigned long long value) noexcept { Tools::Tracy::Plot(name, value); }
    void PlotI(const char* name, int value) noexcept { Tools::Tracy::Plot(name, value); }
    void PlotU(const char* name, unsigned int value) noexcept { Tools::Tracy::Plot(name, value); }
}

export namespace Profiling::Backend::Tracy {
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