module;
module Sys.RenderGraph;

import Lang;
import Cap.Memory;
import Sim.Scene;
import :Types;
import :CommandList;
import :FrameGraph;
import :Simple;

namespace Sys {
    namespace {
        inline UInt32 PassTypeFromName(StringView name) noexcept {
            if (name.size() > 0) {
                Char8 c = name[0];
                if (c == static_cast<Char8>('C')) return static_cast<UInt32>(0);
                if (c == static_cast<Char8>('D')) return static_cast<UInt32>(1);
            }
            return static_cast<UInt32>(0);
        }
    }

    FrameGraph::FrameGraph(Cap::Allocator alloc) noexcept
        : m_alloc(alloc), m_nameCount(0), m_edgeCount(0), m_orderCount(0), m_useCount(0), m_resCount(0), m_aliasGroups(0), m_peakBytes(0), m_simple(), m_cmd(&m_simple) {}

    PassId FrameGraph::AddPass(StringView name, Span<const PassDependency, DynamicExtent> deps) noexcept {
        UInt32 id = static_cast<UInt32>(m_nameCount);
        if (m_nameCount < kMaxPasses) m_names[static_cast<size_t>(m_nameCount++)] = name;
        if (id < kMaxPasses) m_types[static_cast<size_t>(id)] = PassTypeFromName(name);
        for (USize i = 0; i < deps.size(); ++i) {
            const PassDependency& d = deps[i];
            if (m_edgeCount < kMaxEdges) {
                m_edgesFrom[static_cast<size_t>(m_edgeCount)] = d.dependsOn;
                m_edgesTo[static_cast<size_t>(m_edgeCount)] = id;
                m_edgeCount += static_cast<USize>(1);
            }
        }
        return id;
    }

    ResourceId FrameGraph::AddResource(const ResourceDesc& desc) noexcept {
        UInt32 id = static_cast<UInt32>(m_resCount);
        if (m_resCount < kMaxRes) m_res[static_cast<size_t>(m_resCount++)] = desc;
        return id;
    }

    bool FrameGraph::ReadResource(PassId pass, ResourceId res) noexcept {
        if (pass >= static_cast<PassId>(m_nameCount)) return false;
        if (res >= static_cast<ResourceId>(m_resCount)) return false;
        if (m_useCount < kMaxUses) {
            m_usePass[static_cast<size_t>(m_useCount)] = pass;
            m_useRes[static_cast<size_t>(m_useCount)] = res;
            m_useMode[static_cast<size_t>(m_useCount)] = static_cast<UInt32>(0);
            m_useCount += static_cast<USize>(1);
        }
        return true;
    }

    bool FrameGraph::WriteResource(PassId pass, ResourceId res) noexcept {
        if (pass >= static_cast<PassId>(m_nameCount)) return false;
        if (res >= static_cast<ResourceId>(m_resCount)) return false;
        if (m_useCount < kMaxUses) {
            m_usePass[static_cast<size_t>(m_useCount)] = pass;
            m_useRes[static_cast<size_t>(m_useCount)] = res;
            m_useMode[static_cast<size_t>(m_useCount)] = static_cast<UInt32>(1);
            m_useCount += static_cast<USize>(1);
        }
        return true;
    }

    bool FrameGraph::Compile() noexcept {
        USize n = m_nameCount;
        UInt32 indeg[kMaxPasses]{};
        for (USize i = 0; i < n; ++i) indeg[static_cast<size_t>(i)] = 0;
        for (USize e = 0; e < m_edgeCount; ++e) {
            UInt32 to = m_edgesTo[static_cast<size_t>(e)];
            indeg[static_cast<size_t>(to)] += static_cast<UInt32>(1);
        }
        m_orderCount = 0;
        UInt32 cur[kMaxPasses]{}; USize curN = 0;
        for (USize i = 0; i < n; ++i) {
            if (indeg[static_cast<size_t>(i)] == 0) cur[static_cast<size_t>(curN++)] = static_cast<UInt32>(i);
        }
        UInt32 next[kMaxPasses]{}; USize nextN = 0;
        m_levels[0] = 0; USize levelN = 0;
        while (curN > 0) {
            for (USize i = 0; i < curN; ++i) m_order[static_cast<size_t>(m_orderCount++)] = cur[static_cast<size_t>(i)];
            m_levels[static_cast<size_t>(levelN++)] = static_cast<UInt32>(curN);
            nextN = 0;
            for (USize i = 0; i < curN; ++i) {
                UInt32 v = cur[static_cast<size_t>(i)];
                for (USize e = 0; e < m_edgeCount; ++e) {
                    if (m_edgesFrom[static_cast<size_t>(e)] == v) {
                        UInt32 w = m_edgesTo[static_cast<size_t>(e)];
                        UInt32 d = indeg[static_cast<size_t>(w)];
                        if (d > 0) {
                            d -= static_cast<UInt32>(1);
                            indeg[static_cast<size_t>(w)] = d;
                            if (d == 0) next[static_cast<size_t>(nextN++)] = w;
                        }
                    }
                }
            }
            for (USize i = 0; i < nextN; ++i) cur[static_cast<size_t>(i)] = next[static_cast<size_t>(i)];
            curN = nextN;
        }
        if (m_orderCount != n) return false;

        UInt32 topoIndex[kMaxPasses]{};
        for (USize i = 0; i < n; ++i) topoIndex[static_cast<size_t>(m_order[static_cast<size_t>(i)])] = static_cast<UInt32>(i);

        UInt32 start[kMaxRes]{};
        UInt32 end[kMaxRes]{};
        for (USize i = 0; i < m_resCount; ++i) { start[static_cast<size_t>(i)] = static_cast<UInt32>(n); end[static_cast<size_t>(i)] = 0; }
        for (USize i = 0; i < m_useCount; ++i) {
            UInt32 p = m_usePass[static_cast<size_t>(i)];
            UInt32 r = m_useRes[static_cast<size_t>(i)];
            UInt32 t = topoIndex[static_cast<size_t>(p)];
            UInt32 s = start[static_cast<size_t>(r)];
            UInt32 e = end[static_cast<size_t>(r)];
            if (t < s) start[static_cast<size_t>(r)] = t;
            if (t > e) end[static_cast<size_t>(r)] = t;
        }

        UInt32 orderRes[kMaxRes]{}; USize ordN = m_resCount;
        for (USize i = 0; i < ordN; ++i) orderRes[static_cast<size_t>(i)] = static_cast<UInt32>(i);
        for (USize i = 0; i + 1 < ordN; ++i) {
            for (USize j = i + 1; j < ordN; ++j) {
                if (start[static_cast<size_t>(orderRes[static_cast<size_t>(j)])] < start[static_cast<size_t>(orderRes[static_cast<size_t>(i)])]) {
                    UInt32 tmp = orderRes[static_cast<size_t>(i)];
                    orderRes[static_cast<size_t>(i)] = orderRes[static_cast<size_t>(j)];
                    orderRes[static_cast<size_t>(j)] = tmp;
                }
            }
        }

        UInt32 slotEnd[kMaxRes]{}; USize slotN = 0;
        UInt32 resSlot[kMaxRes]{};
        for (USize i = 0; i < m_resCount; ++i) resSlot[static_cast<size_t>(i)] = static_cast<UInt32>(~0u);
        for (USize idx = 0; idx < ordN; ++idx) {
            UInt32 r = orderRes[static_cast<size_t>(idx)];
            UInt32 s = start[static_cast<size_t>(r)];
            UInt32 e = end[static_cast<size_t>(r)];
            UInt32 assigned = static_cast<UInt32>(~0u);
            for (USize k = 0; k < slotN; ++k) {
                UInt32 se = slotEnd[static_cast<size_t>(k)];
                if (se < s) { assigned = static_cast<UInt32>(k); break; }
            }
            if (assigned == static_cast<UInt32>(~0u)) { assigned = static_cast<UInt32>(slotN); slotEnd[static_cast<size_t>(slotN++)] = e; }
            else { slotEnd[static_cast<size_t>(assigned)] = e; }
            resSlot[static_cast<size_t>(r)] = assigned;
        }
        m_aliasGroups = slotN;

        Int64 sweep[kMaxPasses + 1]{};
        for (USize i = 0; i < n + 1; ++i) sweep[static_cast<size_t>(i)] = 0;
        for (USize r = 0; r < m_resCount; ++r) {
            USize bytes = m_res[static_cast<size_t>(r)].bytes;
            UInt32 s = start[static_cast<size_t>(r)];
            UInt32 e = end[static_cast<size_t>(r)];
            if (s <= e && s < static_cast<UInt32>(n)) {
                sweep[static_cast<size_t>(s)] += static_cast<Int64>(bytes);
                sweep[static_cast<size_t>(e + 1)] -= static_cast<Int64>(bytes);
            }
        }
        Int64 curTotal = 0;
        Int64 peakTotal = 0;
        for (USize i = 0; i < n + 1; ++i) { curTotal += sweep[static_cast<size_t>(i)]; if (curTotal > peakTotal) peakTotal = curTotal; }
        m_peakBytes = static_cast<USize>(peakTotal < 0 ? 0 : peakTotal);
        return true;
    }

    bool FrameGraph::Execute(const Sim::Scene::SceneView& view) noexcept {
        (void)view;
        USize idx = 0;
        for (USize li = 0; li < kMaxPasses && m_levels[static_cast<size_t>(li)] != 0; ++li) {
            USize count = m_levels[static_cast<size_t>(li)];
            for (USize k = 0; k < count; ++k) {
                UInt32 pid = m_order[static_cast<size_t>(idx + k)];
                UInt32 pt = m_types[static_cast<size_t>(pid)];
                if (pt == static_cast<UInt32>(0)) {
                    // Collect stage stub
                } else if (pt == static_cast<UInt32>(1)) {
                    m_cmd.Begin();
                    m_cmd.End();
                }
            }
            m_cmd.Flush();
            idx += count;
        }
        return true;
    }

    USize FrameGraph::PassCount() const noexcept { return m_nameCount; }
    USize FrameGraph::DrawCalls() const noexcept { return m_cmd.DrawCalls(); }
    USize FrameGraph::Submissions() const noexcept { return m_cmd.Submissions(); }
    USize FrameGraph::Flushes() const noexcept { return m_cmd.Flushes(); }
    USize FrameGraph::AliasGroups() const noexcept { return m_aliasGroups; }
    USize FrameGraph::PeakBytes() const noexcept { return m_peakBytes; }
    Span<const UInt32, DynamicExtent> FrameGraph::UsagePass() const noexcept { return Span<const UInt32, DynamicExtent>{ m_usePass, m_useCount }; }
    Span<const UInt32, DynamicExtent> FrameGraph::UsageRes() const noexcept { return Span<const UInt32, DynamicExtent>{ m_useRes, m_useCount }; }
    Span<const UInt32, DynamicExtent> FrameGraph::UsageMode() const noexcept { return Span<const UInt32, DynamicExtent>{ m_useMode, m_useCount }; }
    Span<const UInt32, DynamicExtent> FrameGraph::Order() const noexcept { return Span<const UInt32, DynamicExtent>{ m_order, m_orderCount }; }
    USize FrameGraph::ResourceCount() const noexcept { return m_resCount; }
}
