module;
export module Engine.Renderer:FrameGraph;

import Lang;
import Foundation.Memory;
import Foundation.Containers;
import Engine.Scene;
import :Types;
import :CommandList;
import :Simple;

export namespace Engine::Renderer {
    struct ResourceDesc { Language::USize bytes{0}; Language::UInt32 type{0}; };
    using ResourceId = Language::UInt32;
    struct PassDependency { Language::UInt32 dependsOn{0}; };
    using PassId = Language::UInt32;

    class FrameGraph {
    public:
        explicit FrameGraph(::Foundation::Memory::IAllocator* alloc) noexcept;
        PassId AddPass(Language::StringView name, Language::Span<const PassDependency, Language::DynamicExtent> deps) noexcept;
        ResourceId AddResource(const ResourceDesc& desc) noexcept;
        bool ReadResource(PassId pass, ResourceId res) noexcept;
        bool WriteResource(PassId pass, ResourceId res) noexcept;
        bool Compile() noexcept;
        bool Execute(const Engine::Scene::SceneView& view) noexcept;
        Language::USize PassCount() const noexcept;
        Language::USize DrawCalls() const noexcept;
        Language::USize Submissions() const noexcept;
        Language::USize Flushes() const noexcept;
        Language::USize AliasGroups() const noexcept;
        Language::USize PeakBytes() const noexcept;
        Language::Span<const Language::UInt32, Language::DynamicExtent> UsagePass() const noexcept;
        Language::Span<const Language::UInt32, Language::DynamicExtent> UsageRes() const noexcept;
        Language::Span<const Language::UInt32, Language::DynamicExtent> UsageMode() const noexcept;
        Language::Span<const Language::UInt32, Language::DynamicExtent> Order() const noexcept;
        Language::USize ResourceCount() const noexcept;
    private:
        ::Foundation::Memory::IAllocator* m_alloc{};
        ::Foundation::Containers::Vector<Language::StringView> m_names;
        ::Foundation::Containers::Vector<Language::UInt32> m_types;
        ::Foundation::Containers::Vector<Language::UInt32> m_edgesFrom;
        ::Foundation::Containers::Vector<Language::UInt32> m_edgesTo;
        ::Foundation::Containers::Vector<Language::UInt32> m_order;
        ::Foundation::Containers::Vector<ResourceDesc> m_res;
        ::Foundation::Containers::Vector<Language::UInt32> m_usePass;
        ::Foundation::Containers::Vector<Language::UInt32> m_useRes;
        ::Foundation::Containers::Vector<Language::UInt32> m_useMode;
        ::Foundation::Containers::Vector<Language::UInt32> m_levels;
        Language::USize m_aliasGroups{0};
        Language::USize m_peakBytes{0};
        SimpleRenderer m_simple;
        CommandList m_cmd;
    };
}