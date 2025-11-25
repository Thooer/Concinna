module;
export module Engine.Scene:Component;

import Language;
import :Types;
import :World;

export namespace Engine::Scene {
    inline constexpr Language::UInt32 Component_Transform = static_cast<Language::UInt32>(1u);

    template<class T>
    bool AddComponent(SceneWorld& w, Entity e) noexcept;

    template<class T>
    bool RemoveComponent(SceneWorld& w, Entity e) noexcept;

    template<class T>
    bool Has(SceneWorld const& w, Entity e) noexcept;

    template<> inline bool AddComponent<Transform>(SceneWorld& w, Entity e) noexcept { return w.AddTransformComponent(e); }
    template<> inline bool RemoveComponent<Transform>(SceneWorld& w, Entity e) noexcept { return w.RemoveTransformComponent(e); }
    template<> inline bool Has<Transform>(SceneWorld const& w, Entity e) noexcept { return w.HasTransform(e); }
}