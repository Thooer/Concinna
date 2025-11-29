module;
export module Engine.Physics.PhysicsSmoke;

import Lang;
import Engine.Physics;

namespace Nova::Samples::EnginePhysics {
    export bool Run() noexcept {
        Engine::Physics::PhysicsWorld world{};
        Language::UInt32 base = 1;
        for (int z = 0; z < 10; ++z) {
            for (int y = 0; y < 10; ++y) {
                for (int x = 0; x < 10; ++x) {
                    Engine::Physics::Entity e = static_cast<Engine::Physics::Entity>(base++);
                    Engine::Physics::AABB a{};
                    a.min = { static_cast<Language::Float32>(x), static_cast<Language::Float32>(y), static_cast<Language::Float32>(z) };
                    a.max = { static_cast<Language::Float32>(x)+0.5f, static_cast<Language::Float32>(y)+0.5f, static_cast<Language::Float32>(z)+0.5f };
                    if (!world.AddCollider(e, a)) return false;
                }
            }
        }

        Engine::Physics::AABB q{};
        q.min = { 2.0f, 2.0f, 2.0f };
        q.max = { 4.0f, 4.0f, 4.0f };
        auto view = world.QueryAABB(q);
        if (view.count == 0) return false;

        Engine::Physics::RayQuery rq{};
        rq.origin = { 0.0f, 0.0f, 0.0f };
        rq.dir = { 5.0f, 5.0f, 5.0f };
        auto hit = world.Raycast(rq);
        if (!hit.hit) return false;
        return true;
    }
}