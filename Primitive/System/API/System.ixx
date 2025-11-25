export module Prm.System;
export import :Topology;

export namespace Prm {
    enum class OSKind : UInt8 { Unknown = 0, Windows, Linux, MacOS };

    struct KernelInfo {
        OSKind kind{OSKind::Unknown};
        UInt32 major{0};
        UInt32 minor{0};
        UInt32 build{0};
    };

    struct CpuInfo {
        UInt32 logicalCores{0};
        UInt32 physicalCores{0};
        bool   hyperThreading{false};
    };

    struct MemoryInfo {
        UInt64 physicalTotal{0};
        UInt64 physicalAvailable{0};
        UInt64 virtualTotal{0};
        UInt64 virtualAvailable{0};
    };

    struct SystemMemoryInfo {
        UInt64 physicalTotal{0};
        UInt64 physicalAvailable{0};
        UInt64 pageFileTotal{0};
        UInt64 pageFileAvailable{0};
        UInt64 virtualTotal{0};
        UInt64 virtualAvailable{0};
    };

    struct NumaInfo {
        UInt32 highestNode{0};
        UInt32 nodeCount{0};
    };

    class SystemInfo {
    public:
        [[nodiscard]] static KernelInfo Kernel() noexcept;
        [[nodiscard]] static CpuInfo    Cpu() noexcept;
        [[nodiscard]] static MemoryInfo Memory() noexcept;
        [[nodiscard]] static Expect<NumaInfo>         GetNumaInfo() noexcept;
        [[nodiscard]] static Expect<SystemMemoryInfo> QuerySystemMemoryInfo() noexcept;
        [[nodiscard]] static Expect<USize> HostName(Span<Char8, DynamicExtent> buffer) noexcept;
        [[nodiscard]] static Expect<USize> UserName(Span<Char8, DynamicExtent> buffer) noexcept;
    };

    class Device {
    public:
        struct DisplayInfo {
            UInt32 width{0};
            UInt32 height{0};
            UInt32 refreshHz{0};
            bool   primary{true};
        };
        [[nodiscard]] static Expect<USize> EnumerateDisplays(Span<DisplayInfo, DynamicExtent> out) noexcept;
    };

    export [[nodiscard]] USize CaptureStackTrace(Span<void*, DynamicExtent> frameBuffer) noexcept;
}
