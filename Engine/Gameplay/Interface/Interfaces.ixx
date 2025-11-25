module;
export module Engine.Gameplay:Interfaces;

import Language;
import Engine.Scene;

export namespace Engine::Gameplay {
    using ComponentType = Language::UInt32;
    class IComponent {
    public:
        virtual ~IComponent() = default;
        virtual void OnAttach(Engine::Scene::Entity) noexcept {}
        virtual void OnDetach(Engine::Scene::Entity) noexcept {}
        virtual void Tick(float) noexcept {}
    };
    class IGameSystem {
    public:
        virtual ~IGameSystem() = default;
        virtual bool Initialize() noexcept { return true; }
        virtual void Tick(float) noexcept {}
        virtual void Shutdown() noexcept {}
    };
}