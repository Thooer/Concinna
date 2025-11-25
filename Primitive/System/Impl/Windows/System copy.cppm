module;
module Platform;

import Prm;
import :System;

namespace Platform {
    

    // 基本系统信息 API（尽量使用 kernel32，避免额外链接依赖）
    extern "C" __declspec(dllimport) void   GetSystemInfo(void* lpSystemInfo);
    extern "C" __declspec(dllimport) int    GetComputerNameA(char* lpBuffer, unsigned long* lpnSize);
    extern "C" __declspec(dllimport) unsigned long GetActiveProcessorCount(unsigned short groupNumber);
    extern "C" __declspec(dllimport) int    GlobalMemoryStatusEx(void* lpBuffer);
    extern "C" __declspec(dllimport) unsigned long GetVersion(void);
    extern "C" __declspec(dllimport) int    GetNumaHighestNodeNumber(unsigned long* HighestNodeNumber);

    struct SYSTEM_INFO_WIN {
        unsigned short wProcessorArchitecture; // 简化，不完整
        unsigned short wReserved;
        unsigned long  dwPageSize;
        void*          lpMinimumApplicationAddress;
        void*          lpMaximumApplicationAddress;
        unsigned long  dwActiveProcessorMaskLow; // 简化：不完整
        unsigned long  dwNumberOfProcessors;
        unsigned long  dwProcessorType;
        unsigned long  dwAllocationGranularity;
        unsigned short wProcessorLevel;
        unsigned short wProcessorRevision;
    };

    struct MEMORYSTATUSEX_WIN {
        unsigned long dwLength;
        unsigned long dwMemoryLoad;
        UInt64 ullTotalPhys;
        UInt64 ullAvailPhys;
        UInt64 ullTotalPageFile;
        UInt64 ullAvailPageFile;
        UInt64 ullTotalVirtual;
        UInt64 ullAvailVirtual;
        UInt64 ullAvailExtendedVirtual;
    };

    KernelInfo SystemInfo::Kernel() noexcept {
        KernelInfo k{};
        k.kind = OSKind::Windows;
        // GetVersion 返回值高低位编码版本（已弃用，但足以填充占位）
        const unsigned long v = GetVersion();
        k.major = static_cast<UInt32>(v & 0xFFu);
        k.minor = static_cast<UInt32>((v >> 8) & 0xFFu);
        k.build = static_cast<UInt32>((v >> 16) & 0xFFFFu);
        return k;
    }

    CpuInfo SystemInfo::Cpu() noexcept {
        CpuInfo c{};
        SYSTEM_INFO_WIN si{};
        GetSystemInfo(&si);
        c.logicalCores = static_cast<UInt32>(si.dwNumberOfProcessors);
        // 物理核心数估算：未知则与逻辑数相同；不尝试枚举复杂的处理器信息。
        c.physicalCores = c.logicalCores;
        c.hyperThreading = false;
        // 若可用，查询活动处理器数量（按组 0），作为逻辑核心的更稳妥来源。
        const unsigned long active = GetActiveProcessorCount(0);
        if (active != 0) {
            c.logicalCores = static_cast<UInt32>(active);
        }
        return c;
    }

    MemoryInfo SystemInfo::Memory() noexcept {
        MemoryInfo m{};
        MEMORYSTATUSEX_WIN ms{};
        ms.dwLength = sizeof(MEMORYSTATUSEX_WIN);
        const int ok = GlobalMemoryStatusEx(&ms);
        if (ok) {
            m.physicalTotal = static_cast<UInt64>(ms.ullTotalPhys);
            m.physicalAvailable = static_cast<UInt64>(ms.ullAvailPhys);
            m.virtualTotal = static_cast<UInt64>(ms.ullTotalVirtual);
            m.virtualAvailable = static_cast<UInt64>(ms.ullAvailVirtual);
        }
        return m;
    }

    Expect<NumaInfo> SystemInfo::GetNumaInfo() noexcept {
        unsigned long highest = 0;
        const int ok = GetNumaHighestNodeNumber(&highest);
        if (!ok) {
            return Expect<NumaInfo>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        NumaInfo info{};
        info.highestNode = static_cast<UInt32>(highest);
        info.nodeCount = static_cast<UInt32>(highest + 1u);
        return Expect<NumaInfo>::Ok(info);
    }

    Expect<SystemMemoryInfo> SystemInfo::QuerySystemMemoryInfo() noexcept {
        MEMORYSTATUSEX_WIN ms{};
        ms.dwLength = sizeof(MEMORYSTATUSEX_WIN);
        const int ok = GlobalMemoryStatusEx(&ms);
        if (!ok) {
            return Expect<SystemMemoryInfo>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        SystemMemoryInfo info{};
        info.physicalTotal     = static_cast<UInt64>(ms.ullTotalPhys);
        info.physicalAvailable = static_cast<UInt64>(ms.ullAvailPhys);
        info.pageFileTotal     = static_cast<UInt64>(ms.ullTotalPageFile);
        info.pageFileAvailable = static_cast<UInt64>(ms.ullAvailPageFile);
        info.virtualTotal      = static_cast<UInt64>(ms.ullTotalVirtual);
        info.virtualAvailable  = static_cast<UInt64>(ms.ullAvailVirtual);
        return Expect<SystemMemoryInfo>::Ok(info);
    }

    Expect<USize> SystemInfo::HostName(Span<Char8, DynamicExtent> buffer) noexcept {
        if (buffer.empty()) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        unsigned long n = static_cast<unsigned long>(buffer.size());
        const int ok = GetComputerNameA(reinterpret_cast<char*>(buffer.data()), &n);
        if (!ok) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        return Expect<USize>::Ok(static_cast<USize>(n));
    }

    extern "C" __declspec(dllimport) unsigned long GetEnvironmentVariableA(const char* lpName, char* lpBuffer, unsigned long nSize);

    Expect<USize> SystemInfo::UserName(Span<Char8, DynamicExtent> buffer) noexcept {
        if (buffer.empty()) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        // 避免链接 advapi32，使用环境变量 USERNAME 获取当前用户
        const unsigned long n = GetEnvironmentVariableA("USERNAME", reinterpret_cast<char*>(buffer.data()), static_cast<unsigned long>(buffer.size()));
        if (n == 0 || n > buffer.size()) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        return Expect<USize>::Ok(static_cast<USize>(n));
    }

    Expect<USize> Device::EnumerateDisplays(Span<DisplayInfo, DynamicExtent> out) noexcept {
        if (out.size() == 0) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
        }
        // 最小实现：仅报告一个主显示，使用占位分辨率。
        out[0].width = 1280;
        out[0].height = 720;
        out[0].refreshHz = 60;
        out[0].primary = true;
        return Expect<USize>::Ok(static_cast<USize>(1));
    }

    // 调用栈捕获（使用 RtlCaptureStackBackTrace）
    extern "C" __declspec(dllimport) unsigned short RtlCaptureStackBackTrace(unsigned long FramesToSkip, unsigned long FramesToCapture, void** BackTrace, unsigned long* BackTraceHash);

    USize CaptureStackTrace(Span<void*, DynamicExtent> frameBuffer) noexcept {
        if (frameBuffer.empty()) return 0;
        const unsigned long toCapture = static_cast<unsigned long>(frameBuffer.size());
        const unsigned short captured = RtlCaptureStackBackTrace(0, toCapture, frameBuffer.data(), nullptr);
        return static_cast<USize>(captured);
    }
}
