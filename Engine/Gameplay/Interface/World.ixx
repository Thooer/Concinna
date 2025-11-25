module;
export module Engine.Gameplay:World;

import Language;
import Engine.Scene;
import :Interfaces;

export namespace Engine::Gameplay {
    class GameplayWorld {
    public:
        bool RegisterComponent(ComponentType) noexcept { return true; }
        bool Attach(Engine::Scene::Entity, ComponentType) noexcept { return true; }
        bool Detach(Engine::Scene::Entity, ComponentType) noexcept { return true; }
    };
}