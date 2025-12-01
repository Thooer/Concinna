export module Engine.Runtime.IntegrationSmoke;

import Lang;
import Cap.Memory;
import Foundation.Time;
import Engine.Runtime;
import Engine.Scene;
import Engine.Resource;
import Engine.Renderer;

namespace Nova::Samples::EngineIntegration {
    struct ResourceSys : ::Engine::ISystem {
        Engine::Resource::ResourceManager* rm{};
        explicit ResourceSys(Engine::Resource::ResourceManager* m) noexcept : rm(m) {}
        bool Initialize(const ::Engine::CoreConfig&) noexcept { return true; }
        void Tick(float) noexcept {
            const ::Char8* p = reinterpret_cast<const ::Char8*>("dummy");
            ::StringView sv(p, 5);
            (void)rm->LoadMesh(sv);
        }
        void Shutdown() noexcept {}
    };
    struct SceneSys : ::Engine::ISystem {
        Engine::Scene::SceneWorld* sw{}; Cap::IAllocator* alloc{};
        SceneSys(Engine::Scene::SceneWorld* w, Cap::IAllocator* a) noexcept : sw(w), alloc(a) {}
        bool Initialize(const ::Engine::CoreConfig&) noexcept { return true; }
        void Tick(float) noexcept { auto e = sw->CreateEntity(); Engine::Scene::Transform t{}; t.x=1; t.y=2; t.z=3; (void)sw->SetTransform(e, t); }
        void Shutdown() noexcept {}
    };
    struct RendererSys : ::Engine::ISystem {
        Engine::Renderer::SimpleRenderer* r{}; Engine::Scene::SceneWorld* sw{}; Engine::Resource::ResourceManager* rm{};
        RendererSys(Engine::Renderer::SimpleRenderer* rr, Engine::Scene::SceneWorld* w, Engine::Resource::ResourceManager* m) noexcept : r(rr), sw(w), rm(m) {}
        bool Initialize(const ::Engine::CoreConfig&) noexcept { return true; }
        void Tick(float) noexcept {
            auto view = sw->GetView();
            Engine::Renderer::RenderInstance inst{};
            for (USize i=0;i<view.count;++i) { (void)r->Submit(&inst, 1); }
            r->Flush();
        }
        void Shutdown() noexcept {}
    };

    export bool Run() {
        auto allocScoped = Cap::CreateDefaultAllocatorScoped();
        if (!allocScoped.IsOk()) return false; auto alloc = allocScoped.OkValue().Get();
        ::Engine::EngineRuntime eng{}; ::Engine::CoreConfig cfg{}; if (!eng.Initialize(cfg)) return false;
        Engine::Resource::ResourceManager rm{alloc}; Engine::Scene::SceneWorld sw{alloc}; Engine::Renderer::SimpleRenderer r{};
        ResourceSys sR(&rm); SceneSys sS(&sw, alloc); RendererSys sE(&r, &sw, &rm);
        if (!eng.RegisterSystem(&sR, 10)) return false;
        if (!eng.RegisterSystem(&sS, 20)) return false;
        if (!eng.RegisterSystem(&sE, 30)) return false;
        for (int i=0;i<2;++i) { eng.BeginFrame(); eng.Tick(0.016f); eng.EndFrame(); }
        eng.Shutdown();
        return true;
    }
}