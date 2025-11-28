export module Cap.Algorithms:Spatial;

import Language;
import Cap.Math;

export namespace Cap {
    struct LBVHNode {
        Int32 left{-1};
        Int32 right{-1};
        Int32 parent{-1};
        Cap::AABB bbox{};
    };

    inline void BuildLBVH(Span<const UInt32> morton, Span<LBVHNode> nodes) noexcept {
        USize n = morton.size(); if (nodes.size() < n) n = nodes.size();
        for (USize i = 0; i < n; ++i) { nodes[i].left = -1; nodes[i].right = -1; nodes[i].parent = -1; }
        for (USize i = 1; i < n; ++i) { nodes[i-1].right = static_cast<Int32>(i); nodes[i].parent = static_cast<Int32>(i-1); }
    }
}
