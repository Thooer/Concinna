module;
module Engine.Renderer;

import Language;
import Foundation.Memory;
import Foundation.Containers;
import Engine.Scene;
import :Types;
import :CommandList;
import :FrameGraph;
import :Simple;

namespace Engine::Renderer {
    namespace {
        inline Language::UInt32 PassTypeFromName(Language::StringView name) noexcept {
            if (name.size() > 0) {
                Language::Char8 c = name[0];
                if (c == static_cast<Language::Char8>('C')) return static_cast<Language::UInt32>(0);
                if (c == static_cast<Language::Char8>('D')) return static_cast<Language::UInt32>(1);
            }
            return static_cast<Language::UInt32>(0);
        }
    }

    FrameGraph::FrameGraph(::Foundation::Memory::IAllocator* alloc) noexcept
        : m_alloc(alloc), m_names(alloc), m_types(alloc), m_edgesFrom(alloc), m_edgesTo(alloc), m_order(alloc), m_res(alloc), m_usePass(alloc), m_useRes(alloc), m_useMode(alloc), m_levels(alloc), m_aliasGroups(0), m_peakBytes(0), m_simple(), m_cmd(&m_simple) {}

    PassId FrameGraph::AddPass(Language::StringView name, Language::Span<const PassDependency, Language::DynamicExtent> deps) noexcept {
        Language::UInt32 id = static_cast<Language::UInt32>(m_names.size());
        (void)m_names.push_back(name);
        (void)m_types.push_back(PassTypeFromName(name));
        for (Language::USize i = 0; i < deps.size(); ++i) {
            const PassDependency& d = deps[i];
            (void)m_edgesFrom.push_back(d.dependsOn);
            (void)m_edgesTo.push_back(id);
        }
        return id;
    }

    ResourceId FrameGraph::AddResource(const ResourceDesc& desc) noexcept {
        Language::UInt32 id = static_cast<Language::UInt32>(m_res.size());
        (void)m_res.push_back(desc);
        return id;
    }

    bool FrameGraph::ReadResource(PassId pass, ResourceId res) noexcept {
        if (pass >= static_cast<PassId>(m_names.size())) return false;
        if (res >= static_cast<ResourceId>(m_res.size())) return false;
        (void)m_usePass.push_back(pass);
        (void)m_useRes.push_back(res);
        (void)m_useMode.push_back(static_cast<Language::UInt32>(0));
        return true;
    }

    bool FrameGraph::WriteResource(PassId pass, ResourceId res) noexcept {
        if (pass >= static_cast<PassId>(m_names.size())) return false;
        if (res >= static_cast<ResourceId>(m_res.size())) return false;
        (void)m_usePass.push_back(pass);
        (void)m_useRes.push_back(res);
        (void)m_useMode.push_back(static_cast<Language::UInt32>(1));
        return true;
    }

    bool FrameGraph::Compile() noexcept {
        Language::USize n = m_names.size();
        ::Foundation::Containers::Vector<Language::UInt32> indeg(m_alloc);
        if (!indeg.resize(n)) return false;
        for (Language::USize i = 0; i < indeg.size(); ++i) indeg.data()[static_cast<size_t>(i)] = 0;
        for (Language::USize e = 0; e < m_edgesTo.size(); ++e) {
            Language::UInt32 to = m_edgesTo.data()[static_cast<size_t>(e)];
            indeg.data()[static_cast<size_t>(to)] += static_cast<Language::UInt32>(1);
        }
        m_order.clear();
        m_levels.clear();
        ::Foundation::Containers::Vector<Language::UInt32> current(m_alloc);
        for (Language::USize i = 0; i < n; ++i) {
            if (indeg.data()[static_cast<size_t>(i)] == 0) (void)current.push_back(static_cast<Language::UInt32>(i));
        }
        while (current.size() > 0) {
            for (Language::USize i = 0; i < current.size(); ++i) (void)m_order.push_back(current.data()[static_cast<size_t>(i)]);
            (void)m_levels.push_back(static_cast<Language::UInt32>(current.size()));
            ::Foundation::Containers::Vector<Language::UInt32> next(m_alloc);
            for (Language::USize i = 0; i < current.size(); ++i) {
                Language::UInt32 v = current.data()[static_cast<size_t>(i)];
                for (Language::USize e = 0; e < m_edgesTo.size(); ++e) {
                    if (m_edgesFrom.data()[static_cast<size_t>(e)] == v) {
                        Language::UInt32 w = m_edgesTo.data()[static_cast<size_t>(e)];
                        Language::UInt32 d = indeg.data()[static_cast<size_t>(w)];
                        if (d > 0) {
                            d -= static_cast<Language::UInt32>(1);
                            indeg.data()[static_cast<size_t>(w)] = d;
                            if (d == 0) (void)next.push_back(w);
                        }
                    }
                }
            }
            current = next;
        }
        if (m_order.size() != n) return false;

        ::Foundation::Containers::Vector<Language::UInt32> topoIndex(m_alloc);
        if (!topoIndex.resize(n)) return false;
        for (Language::USize i = 0; i < n; ++i) topoIndex.data()[static_cast<size_t>(m_order.data()[static_cast<size_t>(i)])] = static_cast<Language::UInt32>(i);

        ::Foundation::Containers::Vector<Language::UInt32> start(m_alloc);
        ::Foundation::Containers::Vector<Language::UInt32> end(m_alloc);
        if (!start.resize(m_res.size())) return false;
        if (!end.resize(m_res.size())) return false;
        for (Language::USize i = 0; i < m_res.size(); ++i) { start.data()[static_cast<size_t>(i)] = static_cast<Language::UInt32>(n); end.data()[static_cast<size_t>(i)] = 0; }
        for (Language::USize i = 0; i < m_usePass.size(); ++i) {
            Language::UInt32 p = m_usePass.data()[static_cast<size_t>(i)];
            Language::UInt32 r = m_useRes.data()[static_cast<size_t>(i)];
            Language::UInt32 t = topoIndex.data()[static_cast<size_t>(p)];
            Language::UInt32 s = start.data()[static_cast<size_t>(r)];
            Language::UInt32 e = end.data()[static_cast<size_t>(r)];
            if (t < s) start.data()[static_cast<size_t>(r)] = t;
            if (t > e) end.data()[static_cast<size_t>(r)] = t;
        }

        ::Foundation::Containers::Vector<Language::UInt32> orderRes(m_alloc);
        for (Language::USize i = 0; i < m_res.size(); ++i) (void)orderRes.push_back(static_cast<Language::UInt32>(i));
        for (Language::USize i = 0; i + 1 < orderRes.size(); ++i) {
            for (Language::USize j = i + 1; j < orderRes.size(); ++j) {
                if (start.data()[static_cast<size_t>(orderRes.data()[static_cast<size_t>(j)])] < start.data()[static_cast<size_t>(orderRes.data()[static_cast<size_t>(i)])]) {
                    Language::UInt32 tmp = orderRes.data()[static_cast<size_t>(i)];
                    orderRes.data()[static_cast<size_t>(i)] = orderRes.data()[static_cast<size_t>(j)];
                    orderRes.data()[static_cast<size_t>(j)] = tmp;
                }
            }
        }

        ::Foundation::Containers::Vector<Language::UInt32> slotEnd(m_alloc);
        ::Foundation::Containers::Vector<Language::UInt32> resSlot(m_alloc);
        if (!resSlot.resize(m_res.size())) return false;
        for (Language::USize i = 0; i < resSlot.size(); ++i) resSlot.data()[static_cast<size_t>(i)] = static_cast<Language::UInt32>(~0u);
        for (Language::USize idx = 0; idx < orderRes.size(); ++idx) {
            Language::UInt32 r = orderRes.data()[static_cast<size_t>(idx)];
            Language::UInt32 s = start.data()[static_cast<size_t>(r)];
            Language::UInt32 e = end.data()[static_cast<size_t>(r)];
            Language::UInt32 assigned = static_cast<Language::UInt32>(~0u);
            for (Language::USize k = 0; k < slotEnd.size(); ++k) {
                Language::UInt32 se = slotEnd.data()[static_cast<size_t>(k)];
                if (se < s) { assigned = static_cast<Language::UInt32>(k); break; }
            }
            if (assigned == static_cast<Language::UInt32>(~0u)) {
                assigned = static_cast<Language::UInt32>(slotEnd.size());
                (void)slotEnd.push_back(e);
            } else {
                slotEnd.data()[static_cast<size_t>(assigned)] = e;
            }
            resSlot.data()[static_cast<size_t>(r)] = assigned;
        }
        m_aliasGroups = slotEnd.size();

        ::Foundation::Containers::Vector<Language::Int64> sweep(m_alloc);
        if (!sweep.resize(n + 1)) return false;
        for (Language::USize i = 0; i < sweep.size(); ++i) sweep.data()[static_cast<size_t>(i)] = 0;
        for (Language::USize r = 0; r < m_res.size(); ++r) {
            Language::USize bytes = m_res.data()[static_cast<size_t>(r)].bytes;
            Language::UInt32 s = start.data()[static_cast<size_t>(r)];
            Language::UInt32 e = end.data()[static_cast<size_t>(r)];
            if (s <= e && s < static_cast<Language::UInt32>(n)) {
                sweep.data()[static_cast<size_t>(s)] += static_cast<Language::Int64>(bytes);
                sweep.data()[static_cast<size_t>(e + 1)] -= static_cast<Language::Int64>(bytes);
            }
        }
        Language::Int64 cur = 0;
        Language::Int64 peak = 0;
        for (Language::USize i = 0; i < sweep.size(); ++i) { cur += sweep.data()[static_cast<size_t>(i)]; if (cur > peak) peak = cur; }
        m_peakBytes = static_cast<Language::USize>(peak < 0 ? 0 : peak);
        return true;
    }

    bool FrameGraph::Execute(const Engine::Scene::SceneView& view) noexcept {
        ::Foundation::Containers::Vector<RenderInstance> instances(m_alloc);
        Language::USize idx = 0;
        for (Language::USize li = 0; li < m_levels.size(); ++li) {
            Language::USize count = m_levels.data()[static_cast<size_t>(li)];
            for (Language::USize k = 0; k < count; ++k) {
                Language::UInt32 pid = m_order.data()[static_cast<size_t>(idx + k)];
                Language::UInt32 pt = m_types.data()[static_cast<size_t>(pid)];
                if (pt == static_cast<Language::UInt32>(0)) {
                    if (!instances.resize(view.count)) return false;
                    for (Language::USize j = 0; j < view.count; ++j) {
                        RenderInstance ri{};
                        for (int m = 0; m < 16; ++m) ri.model[static_cast<size_t>(m)] = 0.0f;
                        ri.model[0] = 1.0f; ri.model[5] = 1.0f; ri.model[10] = 1.0f; ri.model[15] = 1.0f;
                        ri.model[12] = view.positions[static_cast<size_t>(j*3 + 0)];
                        ri.model[13] = view.positions[static_cast<size_t>(j*3 + 1)];
                        ri.model[14] = view.positions[static_cast<size_t>(j*3 + 2)];
                        ri.mesh = view.meshes[static_cast<size_t>(j)];
                        ri.mat = view.materials[static_cast<size_t>(j)];
                        instances.data()[static_cast<size_t>(j)] = ri;
                    }
                } else if (pt == static_cast<Language::UInt32>(1)) {
                    m_cmd.Begin();
                    m_cmd.Draw(instances.data(), instances.size());
                    m_cmd.End();
                }
            }
            m_cmd.Flush();
            idx += count;
        }
        return true;
    }

    Language::USize FrameGraph::PassCount() const noexcept { return m_names.size(); }
    Language::USize FrameGraph::DrawCalls() const noexcept { return m_cmd.DrawCalls(); }
    Language::USize FrameGraph::Submissions() const noexcept { return m_cmd.Submissions(); }
    Language::USize FrameGraph::Flushes() const noexcept { return m_cmd.Flushes(); }
    Language::USize FrameGraph::AliasGroups() const noexcept { return m_aliasGroups; }
    Language::USize FrameGraph::PeakBytes() const noexcept { return m_peakBytes; }
    Language::Span<const Language::UInt32, Language::DynamicExtent> FrameGraph::UsagePass() const noexcept { return Language::Span<const Language::UInt32, Language::DynamicExtent>{ m_usePass.data(), m_usePass.size() }; }
    Language::Span<const Language::UInt32, Language::DynamicExtent> FrameGraph::UsageRes() const noexcept { return Language::Span<const Language::UInt32, Language::DynamicExtent>{ m_useRes.data(), m_useRes.size() }; }
    Language::Span<const Language::UInt32, Language::DynamicExtent> FrameGraph::UsageMode() const noexcept { return Language::Span<const Language::UInt32, Language::DynamicExtent>{ m_useMode.data(), m_useMode.size() }; }
    Language::Span<const Language::UInt32, Language::DynamicExtent> FrameGraph::Order() const noexcept { return Language::Span<const Language::UInt32, Language::DynamicExtent>{ m_order.data(), m_order.size() }; }
    Language::USize FrameGraph::ResourceCount() const noexcept { return m_res.size(); }
}