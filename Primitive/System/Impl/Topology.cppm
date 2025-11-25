module Prm.System;

import Prm.Ownership;
import :System;
import :Topology;

namespace Prm {
    Topology Detect() noexcept {
        Topology t{};
        auto c = SystemInfo::Cpu();
        t.logicalCores = c.logicalCores;
        t.physicalCores = c.physicalCores ? c.physicalCores : c.logicalCores;
        if (t.physicalCores && t.logicalCores && t.logicalCores >= t.physicalCores) {
            auto r = t.logicalCores / t.physicalCores;
            t.threadsPerCore = r ? r : 1;
        } else {
            t.threadsPerCore = 1;
        }
        auto n = SystemInfo::GetNumaInfo();
        if (n.IsOk()) {
            t.numaNodes = n.Value().nodeCount ? n.Value().nodeCount : 1u;
        } else {
            t.numaNodes = 1;
        }
        return t;
    }

    extern "C" __declspec(dllimport) int GetLogicalProcessorInformationEx(int RelationshipType, void* Buffer, unsigned long* ReturnedLength);

    struct GROUP_AFFINITY_WIN { unsigned long long Mask; unsigned short Group; unsigned short Reserved[3]; };
    struct PROCESSOR_RELATIONSHIP_WIN {
        unsigned char Flags;
        unsigned char EfficiencyClass;
        unsigned char Reserved[21];
        unsigned short GroupCount;
        GROUP_AFFINITY_WIN Groups[1];
    };
    struct SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX_WIN {
        int Relationship;
        unsigned long Size;
        union { PROCESSOR_RELATIONSHIP_WIN Processor; } DUMMYUNIONNAME;
    };

    struct NUMA_NODE_RELATIONSHIP_WIN { unsigned long NodeNumber; unsigned char Reserved[20]; unsigned short GroupCount; GROUP_AFFINITY_WIN GroupMask[1]; };
    struct SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX_NUMA_WIN { int Relationship; unsigned long Size; union { NUMA_NODE_RELATIONSHIP_WIN NumaNode; } DUMMYUNIONNAME; };
    struct CACHE_RELATIONSHIP_WIN { unsigned char Level; unsigned char Type; unsigned short Reserved; unsigned long Size; unsigned short GroupCount; GROUP_AFFINITY_WIN GroupMask[1]; };
    struct SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX_CACHE_WIN { int Relationship; unsigned long Size; union { CACHE_RELATIONSHIP_WIN Cache; } DUMMYUNIONNAME; };
    struct PROCESSOR_PACKAGE_RELATIONSHIP_WIN { unsigned long PackageId; unsigned long Reserved; unsigned short GroupCount; GROUP_AFFINITY_WIN GroupMask[1]; };
    struct SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX_PACKAGE_WIN { int Relationship; unsigned long Size; union { PROCESSOR_PACKAGE_RELATIONSHIP_WIN Package; } DUMMYUNIONNAME; };

    CoreMasks EnumerateCoreMasks() noexcept {
        CoreMasks out{};
        auto topo = Detect();
        UInt32 lc = topo.logicalCores ? topo.logicalCores : 1u;
        UInt32 tpc = topo.threadsPerCore ? topo.threadsPerCore : 1u;
        UInt32 pc = lc / tpc; if (pc == 0u) pc = lc;
        auto hb = Heap::GetProcessDefault();
        auto rc = Heap::AllocRaw(hb, sizeof(CoreMask) * pc);
        if (!rc.IsOk()) return out;
        auto* arr = static_cast<CoreMask*>(rc.Value());
        for (UInt32 i = 0; i < pc; ++i) {
            arr[i].group = 0u;
            UInt64 m = 0u;
            for (UInt32 k = 0; k < tpc; ++k) {
                UInt32 bit = i * tpc + k;
                if (bit < 64u) m |= (UInt64(1) << bit);
            }
            arr[i].mask = m;
        }
        out.data = arr; out.count = pc; return out;
    }

    void Release(CoreMasks cms) noexcept {
        if (!cms.data) return;
        auto hb = Heap::GetProcessDefault();
        (void)Heap::FreeRaw(hb, cms.data);
    }

    NodeMasks EnumerateNumaNodeMasks() noexcept {
        NodeMasks out{};
        auto topo = Detect();
        UInt32 lc = topo.logicalCores ? topo.logicalCores : 1u;
        UInt32 nodes = topo.numaNodes ? topo.numaNodes : 1u;
        auto hb = Heap::GetProcessDefault();
        auto rc = Heap::AllocRaw(hb, sizeof(NodeMask) * nodes);
        if (!rc.IsOk()) return out;
        auto* arr = static_cast<NodeMask*>(rc.Value());
        UInt32 per = lc / nodes; if (per == 0u) per = lc;
        for (UInt32 i = 0; i < nodes; ++i) {
            arr[i].node = i; arr[i].group = 0u; UInt64 m = 0u;
            UInt32 begin = i * per; UInt32 end = begin + per; if (end > lc) end = lc;
            for (UInt32 b = begin; b < end && b < 64u; ++b) { m |= (UInt64(1) << b); }
            arr[i].mask = m;
        }
        out.data = arr; out.count = nodes; return out;
    }

    void Release(NodeMasks nms) noexcept {
        if (!nms.data) return;
        auto hb = Heap::GetProcessDefault();
        (void)Heap::FreeRaw(hb, nms.data);
    }

    CacheMasks EnumerateCacheMasks() noexcept {
        CacheMasks out{};
        auto topo = Detect();
        UInt32 lc = topo.logicalCores ? topo.logicalCores : 1u;
        auto hb = Heap::GetProcessDefault();
        auto rc = Heap::AllocRaw(hb, sizeof(CacheMask));
        if (!rc.IsOk()) return out;
        auto* arr = static_cast<CacheMask*>(rc.Value());
        arr[0].level = 3u; arr[0].id = 0u; arr[0].group = 0u;
        UInt64 m = 0u; for (UInt32 b = 0; b < lc && b < 64u; ++b) { m |= (UInt64(1) << b); }
        arr[0].mask = m;
        out.data = arr; out.count = 1u; return out;
    }

    void Release(CacheMasks cms) noexcept {
        if (!cms.data) return;
        auto hb = Heap::GetProcessDefault();
        (void)Heap::FreeRaw(hb, cms.data);
    }

    PackageMasks EnumeratePackageMasks() noexcept {
        PackageMasks out{};
        auto topo = Detect();
        UInt32 lc = topo.logicalCores ? topo.logicalCores : 1u;
        auto hb = Heap::GetProcessDefault();
        auto rc = Heap::AllocRaw(hb, sizeof(PackageMask));
        if (!rc.IsOk()) return out;
        auto* arr = static_cast<PackageMask*>(rc.Value());
        arr[0].id = 0u; arr[0].group = 0u; UInt64 m = 0u; for (UInt32 b = 0; b < lc && b < 64u; ++b) { m |= (UInt64(1) << b); }
        arr[0].mask = m; out.data = arr; out.count = 1u; return out;
    }

    void Release(PackageMasks pms) noexcept {
        if (!pms.data) return;
        auto hb = Heap::GetProcessDefault();
        (void)Heap::FreeRaw(hb, pms.data);
    }
}
