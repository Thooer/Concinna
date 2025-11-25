module;
module Prm.Time;

import :Time;

namespace Prm {
    

    // Win32 高精度计时 API（避免包含 windows.h）
    extern "C" __declspec(dllimport) int QueryPerformanceFrequency(Int64* lpFrequency);
    extern "C" __declspec(dllimport) int QueryPerformanceCounter(Int64* lpPerformanceCount);
    extern "C" __declspec(dllimport) void Sleep(unsigned long dwMilliseconds);

    // 频率缓存：首次调用 Now() 时查询并缓存
    static Int64 g_qpcFreq = 0; // ticks per second

    void Init() noexcept {
        if (g_qpcFreq != 0) return;
        Int64 f = 0;
        const int ok = QueryPerformanceFrequency(&f);
        g_qpcFreq = ok && f > 0 ? f : 1;
    }

    static inline Int64 QpcNow() noexcept {
        Int64 t = 0;
        QueryPerformanceCounter(&t);
        return t;
    }

    TimePoint Now() noexcept {
        if (g_qpcFreq == 0) {
            Int64 f = 0;
            const int ok = QueryPerformanceFrequency(&f);
            g_qpcFreq = ok && f > 0 ? f : 1; // 退化：避免除零
        }
        const Int64 t = QpcNow();
        // 转换为纳秒： (t * 1e9) / freq
        const Int64 ns = static_cast<Int64>((t * 1'000'000'000ll) / g_qpcFreq);
        return static_cast<Time::TimePoint>(ns);
    }

    void SleepMs(UInt32 milliseconds) noexcept {
        Sleep(static_cast<unsigned long>(milliseconds));
    }

    


    // WallClock: 使用系统挂钟时间
    extern "C" __declspec(dllimport) void GetLocalTime(void* lpSystemTime);
    extern "C" __declspec(dllimport) void GetSystemTime(void* lpSystemTime);

    struct SYSTEMTIME_WIN {
        unsigned short wYear;
        unsigned short wMonth;
        unsigned short wDayOfWeek;
        unsigned short wDay;
        unsigned short wHour;
        unsigned short wMinute;
        unsigned short wSecond;
        unsigned short wMilliseconds;
    };

    WallClock::Data WallClock::Now() noexcept {
        SYSTEMTIME_WIN st{};
        GetLocalTime(&st);
        return WallClock::Data{static_cast<UInt16>(st.wYear), static_cast<UInt8>(st.wMonth), static_cast<UInt8>(st.wDay),
                               static_cast<UInt8>(st.wHour), static_cast<UInt8>(st.wMinute), static_cast<UInt8>(st.wSecond), static_cast<UInt16>(st.wMilliseconds)};
    }

    WallClock::Data WallClock::UtcNow() noexcept {
        SYSTEMTIME_WIN st{};
        GetSystemTime(&st);
        return WallClock::Data{static_cast<UInt16>(st.wYear), static_cast<UInt8>(st.wMonth), static_cast<UInt8>(st.wDay),
                               static_cast<UInt8>(st.wHour), static_cast<UInt8>(st.wMinute), static_cast<UInt8>(st.wSecond), static_cast<UInt16>(st.wMilliseconds)};
    }
}
