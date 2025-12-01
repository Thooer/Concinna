module;
export module Engine.Scene:Types;

import Lang;

export namespace Engine::Scene {
    using Entity = UInt32;
    struct Transform { float x{0}, y{0}, z{0}; };
    struct SceneView {
        const Entity* entities{};
        const float* positions{};
        const UInt32* meshes{};
        const UInt32* materials{};
        USize count{0};
    };
}