export module Prm.System:Topology;
import Element;

export namespace Prm {
    struct Topology {
        UInt32 logicalCores{0};
        UInt32 physicalCores{0};
        UInt32 threadsPerCore{1};
        UInt32 numaNodes{1};
    };

    export Topology Detect() noexcept;

    struct CoreMask {
        UInt32 group{0};
        UInt64 mask{0};
    };
    struct CoreMasks {
        CoreMask* data{nullptr};
        USize count{0};
    };
    export CoreMasks EnumerateCoreMasks() noexcept;
    export void Release(CoreMasks cms) noexcept;

    struct NodeMask { UInt32 node{0}; UInt32 group{0}; UInt64 mask{0}; };
    struct NodeMasks { NodeMask* data{nullptr}; USize count{0}; };
    export NodeMasks EnumerateNumaNodeMasks() noexcept;
    export void Release(NodeMasks nms) noexcept;

    struct CacheMask { UInt32 level{0}; UInt32 id{0}; UInt32 group{0}; UInt64 mask{0}; };
    struct CacheMasks { CacheMask* data{nullptr}; USize count{0}; };
    export CacheMasks EnumerateCacheMasks() noexcept;
    export void Release(CacheMasks cms) noexcept;

    struct PackageMask { UInt32 id{0}; UInt32 group{0}; UInt64 mask{0}; };
    struct PackageMasks { PackageMask* data{nullptr}; USize count{0}; };
    export PackageMasks EnumeratePackageMasks() noexcept;
    export void Release(PackageMasks pms) noexcept;
}
