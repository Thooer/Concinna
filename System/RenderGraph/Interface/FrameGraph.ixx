
module;
export module Sys.RenderGraph:FrameGraph;

import Lang;
import Cap.Memory;
import :Types;
import :CommandList;
import :Simple;

namespace Sim::Scene { struct SceneView; }

export namespace Sys {
    struct ResourceDesc { USize bytes{0}; UInt32 type{0}; };
    using ResourceId = UInt32;
    struct PassDependency { UInt32 dependsOn{0}; };
    using PassId = UInt32;

    class FrameGraph {
    public:
        explicit FrameGraph(Cap::Allocator alloc) noexcept;
        PassId AddPass(StringView name, Span<const PassDependency, DynamicExtent> deps) noexcept;
        ResourceId AddResource(const ResourceDesc& desc) noexcept;
        bool ReadResource(PassId pass, ResourceId res) noexcept;
        bool WriteResource(PassId pass, ResourceId res) noexcept;
        bool Compile() noexcept;
        bool Execute(const Sim::Scene::SceneView& view) noexcept;
        USize PassCount() const noexcept;
        USize DrawCalls() const noexcept;
        USize Submissions() const noexcept;
        USize Flushes() const noexcept;
        USize AliasGroups() const noexcept;
        USize PeakBytes() const noexcept;
        Span<const UInt32, DynamicExtent> UsagePass() const noexcept;
        Span<const UInt32, DynamicExtent> UsageRes() const noexcept;
        Span<const UInt32, DynamicExtent> UsageMode() const noexcept;
        Span<const UInt32, DynamicExtent> Order() const noexcept;
        USize ResourceCount() const noexcept;
    private:
        Cap::Allocator m_alloc{};
        static constexpr USize kMaxPasses = 128;
        static constexpr USize kMaxEdges = 512;
        static constexpr USize kMaxUses = 1024;
        static constexpr USize kMaxRes   = 256;
        StringView m_names[kMaxPasses]{};
        UInt32 m_types[kMaxPasses]{};
        UInt32 m_edgesFrom[kMaxEdges]{};
        UInt32 m_edgesTo[kMaxEdges]{};
        UInt32 m_order[kMaxPasses]{};
        ResourceDesc m_res[kMaxRes]{};
        UInt32 m_usePass[kMaxUses]{};
        UInt32 m_useRes[kMaxUses]{};
        UInt32 m_useMode[kMaxUses]{};
        UInt32 m_levels[kMaxPasses]{};
        USize m_nameCount{0};
        USize m_edgeCount{0};
        USize m_orderCount{0};
        USize m_useCount{0};
        USize m_resCount{0};
        USize m_aliasGroups{0};
        USize m_peakBytes{0};
        SimpleRenderer m_simple;
        CommandList m_cmd;
    };
}