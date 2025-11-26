module Tools.Tracy;
import Tools.Tracy;

#include "Tracy.hpp"

namespace Tools::Tracy {
    Zone::Zone(const char* name) noexcept { p = static_cast<void*>(new tracy::Zone(name)); }
    Zone::~Zone() noexcept { delete static_cast<tracy::Zone*>(p); p = nullptr; }
    void SetThreadName(const char* name) noexcept { TRACY_SET_THREAD_NAME(name); }
    void FrameMark() noexcept { TRACY_FRAME_MARK(); }
    void FrameMarkStart(const char* name) noexcept { TRACY_FRAME_MARK_START(name); }
    void FrameMarkEnd(const char* name) noexcept { TRACY_FRAME_MARK_END(name); }
    void Message(const char* text) noexcept { TRACY_MESSAGE(text); }
    void Plot(const char* name, double value) noexcept { tracy::Plot(name, value); }
    void Plot(const char* name, float value) noexcept { tracy::Plot(name, value); }
    void Plot(const char* name, long long value) noexcept { tracy::Plot(name, static_cast<double>(value)); }
    void Plot(const char* name, unsigned long long value) noexcept { tracy::Plot(name, static_cast<double>(value)); }
    void Plot(const char* name, int value) noexcept { tracy::Plot(name, static_cast<double>(value)); }
    void Plot(const char* name, unsigned int value) noexcept { tracy::Plot(name, static_cast<double>(value)); }
}