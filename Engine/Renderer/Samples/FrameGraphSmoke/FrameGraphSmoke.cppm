module;
export module Engine.Renderer.FrameGraphSmoke;

import Lang;
import Cap.Memory;
import Engine.Scene;
import Engine.Renderer;

namespace Nova::Samples::EngineRenderer {
    export bool Run() noexcept {
        auto ar = Cap::CreateLinearAllocator(static_cast<USize>(1ull << 20));
        if (!ar.IsOk()) return false;
        Cap::IAllocator* alloc = ar.OkValue();

        ::Engine::Scene::SceneWorld world(alloc);
        auto e1 = world.CreateEntity();
        auto e2 = world.CreateEntity();
        (void)world.SetTransform(e1, ::Engine::Scene::Transform{1.0f, 2.0f, 3.0f});
        (void)world.SetTransform(e2, ::Engine::Scene::Transform{4.0f, 5.0f, 6.0f});
        (void)world.BindMesh(e1, static_cast<UInt32>(1));
        (void)world.BindMaterial(e1, static_cast<UInt32>(10));
        (void)world.BindMesh(e2, static_cast<UInt32>(2));
        (void)world.BindMaterial(e2, static_cast<UInt32>(20));

        auto view = world.GetView();

        bool ok = false;
        {
            ::Engine::Renderer::FrameGraph fg(alloc);
            ::Engine::Renderer::PassId pCollect = fg.AddPass(StringView{"Collect"}, Span<const ::Engine::Renderer::PassDependency, DynamicExtent>{});
            ::Engine::Renderer::PassDependency dep{ pCollect };
            ::Engine::Renderer::PassId pDraw = fg.AddPass(StringView{"Draw"}, Span<const ::Engine::Renderer::PassDependency, DynamicExtent>(&dep, 1));
            if (!fg.Compile()) { delete alloc; return false; }
            if (!fg.Execute(view)) { delete alloc; return false; }
            ok = (fg.Submissions() == view.count) && (fg.DrawCalls() == static_cast<USize>(1)) && (fg.Flushes() == static_cast<USize>(2));
        }
        delete alloc;
        return ok;
    }
}