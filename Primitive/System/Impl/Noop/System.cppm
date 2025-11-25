module System;

import Prm;
import System;
import Platform;

namespace System {
    KernelInfo SystemInfo::Kernel() noexcept { auto k = Platform::SystemInfo::Kernel(); KernelInfo r{}; r.kind = static_cast<OSKind>(static_cast<int>(k.kind)); r.major = k.major; r.minor = k.minor; r.build = k.build; return r; }
    CpuInfo    SystemInfo::Cpu() noexcept { auto c = Platform::SystemInfo::Cpu(); CpuInfo r{}; r.logicalCores = c.logicalCores; r.physicalCores = c.physicalCores; r.hyperThreading = c.hyperThreading; return r; }
    MemoryInfo SystemInfo::Memory() noexcept { auto m = Platform::SystemInfo::Memory(); MemoryInfo r{}; r.physicalTotal = m.physicalTotal; r.physicalAvailable = m.physicalAvailable; r.virtualTotal = m.virtualTotal; r.virtualAvailable = m.virtualAvailable; return r; }
    Expect<NumaInfo> SystemInfo::GetNumaInfo() noexcept { auto n = Platform::SystemInfo::GetNumaInfo(); if (!n.IsOk()) return Expect<NumaInfo>::Err(n.Error()); NumaInfo r{}; r.highestNode = n.Value().highestNode; r.nodeCount = n.Value().nodeCount; return Expect<NumaInfo>::Ok(r); }
    Expect<SystemMemoryInfo> SystemInfo::QuerySystemMemoryInfo() noexcept { auto s = Platform::SystemInfo::QuerySystemMemoryInfo(); if (!s.IsOk()) return Expect<SystemMemoryInfo>::Err(s.Error()); SystemMemoryInfo r{}; r.physicalTotal = s.Value().physicalTotal; r.physicalAvailable = s.Value().physicalAvailable; r.pageFileTotal = s.Value().pageFileTotal; r.pageFileAvailable = s.Value().pageFileAvailable; r.virtualTotal = s.Value().virtualTotal; r.virtualAvailable = s.Value().virtualAvailable; return Expect<SystemMemoryInfo>::Ok(r); }
    Expect<USize> SystemInfo::HostName(Span<Char8, DynamicExtent> b) noexcept { return Platform::SystemInfo::HostName(b); }
    Expect<USize> SystemInfo::UserName(Span<Char8, DynamicExtent> b) noexcept { return Platform::SystemInfo::UserName(b); }
    Expect<USize> Device::EnumerateDisplays(Span<Device::DisplayInfo, DynamicExtent> out) noexcept { if (out.size()==0) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange)); struct PDisp { UInt32 w; UInt32 h; UInt32 hz; bool primary; }; Span<PDisp, DynamicExtent> tmp(reinterpret_cast<PDisp*>(out.data()), out.size()); auto r = Platform::Device::EnumerateDisplays(reinterpret_cast<Span<Platform::Device::DisplayInfo, DynamicExtent>&>(tmp)); if (!r.IsOk()) return Expect<USize>::Err(r.Error()); auto cnt = r.Value(); for (USize i=0;i<cnt&&i<out.size();++i){ out[i].width=tmp[i].w; out[i].height=tmp[i].h; out[i].refreshHz=tmp[i].hz; out[i].primary=tmp[i].primary;} return Expect<USize>::Ok(cnt); }
    USize CaptureStackTrace(Span<void*, DynamicExtent> fb) noexcept { return Platform::CaptureStackTrace(fb); }
}