import Prm.System;
import Flow;
import Element;

namespace Prm {
    KernelInfo SystemInfo::Kernel() noexcept { KernelInfo r{}; return r; }

    CpuInfo SystemInfo::Cpu() noexcept { CpuInfo r{}; return r; }

    MemoryInfo SystemInfo::Memory() noexcept { MemoryInfo r{}; return r; }

    Expect<NumaInfo> SystemInfo::GetNumaInfo() noexcept { return Expect<NumaInfo>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }

    Expect<SystemMemoryInfo> SystemInfo::QuerySystemMemoryInfo() noexcept { return Expect<SystemMemoryInfo>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }

    Expect<USize> SystemInfo::HostName(Span<Char8, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
    Expect<USize> SystemInfo::UserName(Span<Char8, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }

    Expect<USize> Device::EnumerateDisplays(Span<Device::DisplayInfo, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }

    USize CaptureStackTrace(Span<void*, DynamicExtent>) noexcept { return 0; }
}
