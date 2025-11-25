module;
export module Engine.Renderer.FrameGraphAsyncAliasSmoke;

import Language;
import Foundation.Memory;
import Engine.Scene;
import Engine.Renderer;

namespace Nova::Samples::FrameGraphAA {
  export bool Run() noexcept {
    auto ar = ::Foundation::Memory::CreateLinearAllocator(static_cast<Language::USize>(1ull << 20));
    if (!ar.IsOk()) return false;
    ::Foundation::Memory::IAllocator* alloc = ar.OkValue();

    ::Engine::Scene::SceneWorld world(alloc);
    auto e1 = world.CreateEntity();
    auto e2 = world.CreateEntity();
    (void)world.SetTransform(e1, ::Engine::Scene::Transform{0.0f, 0.0f, 0.0f});
    (void)world.SetTransform(e2, ::Engine::Scene::Transform{1.0f, 2.0f, 3.0f});
    (void)world.BindMesh(e1, static_cast<Language::UInt32>(1));
    (void)world.BindMaterial(e1, static_cast<Language::UInt32>(10));
    (void)world.BindMesh(e2, static_cast<Language::UInt32>(2));
    (void)world.BindMaterial(e2, static_cast<Language::UInt32>(20));
    auto view = world.GetView();

    bool ok = false;
    {
    ::Engine::Renderer::FrameGraph fg(alloc);
    ::Engine::Renderer::PassId pC1 = fg.AddPass(Language::StringView{"Collect"}, Language::Span<const ::Engine::Renderer::PassDependency, Language::DynamicExtent>{});
    ::Engine::Renderer::PassDependency dC1{ pC1 };
    ::Engine::Renderer::PassId pD1 = fg.AddPass(Language::StringView{"Draw"}, Language::Span<const ::Engine::Renderer::PassDependency, Language::DynamicExtent>(&dC1, 1));
    ::Engine::Renderer::PassDependency dD1{ pD1 };
    ::Engine::Renderer::PassId pP1 = fg.AddPass(Language::StringView{"Collect"}, Language::Span<const ::Engine::Renderer::PassDependency, Language::DynamicExtent>(&dD1, 1));
    ::Engine::Renderer::PassDependency dP1{ pP1 };
    ::Engine::Renderer::PassId pC2 = fg.AddPass(Language::StringView{"Collect"}, Language::Span<const ::Engine::Renderer::PassDependency, Language::DynamicExtent>(&dP1, 1));
    ::Engine::Renderer::PassDependency dC2{ pC2 };
    ::Engine::Renderer::PassId pD2 = fg.AddPass(Language::StringView{"Draw"}, Language::Span<const ::Engine::Renderer::PassDependency, Language::DynamicExtent>(&dC2, 1));
    ::Engine::Renderer::PassDependency dD2{ pD2 };
    ::Engine::Renderer::PassId pP2 = fg.AddPass(Language::StringView{"Collect"}, Language::Span<const ::Engine::Renderer::PassDependency, Language::DynamicExtent>(&dD2, 1));

    ::Engine::Renderer::ResourceDesc rdesc{ static_cast<Language::USize>(8ull << 20), static_cast<Language::UInt32>(0) };
    ::Engine::Renderer::ResourceId rA = fg.AddResource(rdesc);
    ::Engine::Renderer::ResourceId rB = fg.AddResource(rdesc);
    (void)fg.WriteResource(pD1, rA);
    (void)fg.ReadResource(pP1, rA);
    (void)fg.WriteResource(pD2, rB);
    (void)fg.ReadResource(pP2, rB);

    if (!fg.Compile()) { delete alloc; return false; }
    if (!fg.Execute(view)) { delete alloc; return false; }
    ok = (fg.AliasGroups() == static_cast<Language::USize>(1)) && (fg.PeakBytes() == static_cast<Language::USize>(8ull << 20));
    }
    delete alloc;
    return ok;
  }
}