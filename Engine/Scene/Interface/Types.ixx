module;
export module Engine.Scene:Types;

import Lang;

export namespace Engine::Scene {
    using Entity = Language::UInt32;
    struct Transform { float x{0}, y{0}, z{0}; };
    struct SceneView {
        const Entity* entities{};
        const float* positions{};
        const Language::UInt32* meshes{};
        const Language::UInt32* materials{};
        Language::USize count{0};
    };
}