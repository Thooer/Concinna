module;
export module Engine.Scene:Transform;

import Language;
import :Types;
import :World;

export namespace Engine::Scene {
    export Language::StatusResult<Transform> GetTransform(SceneWorld const& w, Entity e) noexcept { return w.GetTransform(e); }
    export bool SetTransform(SceneWorld& w, Entity e, const Transform& t) noexcept { return w.SetTransform(e, t); }
    export bool SetParent(SceneWorld& w, Entity child, Entity parent) noexcept { return w.SetParent(child, parent); }
}