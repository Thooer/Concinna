module;
export module Engine.Physics:Types;

import Language;
// no Engine.Scene dependency to avoid build cycle

export namespace Engine::Physics {
    using Entity = Language::UInt32;
    struct Vec3 { Language::Float32 x{}, y{}, z{}; };
    struct AABB { Vec3 min{}, max{}; };
    struct RayQuery { Vec3 origin{}, dir{}; };
    struct Hit { Entity entity{0}; Vec3 pos{}; bool hit{false}; };

    using ColliderAABB = AABB;

    struct Collider { Entity entity{0}; AABB bounds{}; };

    struct BroadPhaseView {
        const Entity* entities{};
        Language::USize count{0};
    };
}