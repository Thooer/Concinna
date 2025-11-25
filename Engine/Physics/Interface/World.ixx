module;
export module Engine.Physics:World;

import Language;
import :Types;
import Foundation.Containers;
import :Collision;
import :BroadPhase;
import :Raycast;

export namespace Engine::Physics {
    export class PhysicsWorld {
    public:
        bool AddCollider(Entity e, const AABB& a) noexcept { return ::Engine::Physics::AddCollider(e, a); }
        bool RemoveCollider(Entity e) noexcept { return ::Engine::Physics::Remove(e); }
        bool Update(Entity e, const AABB& a) noexcept { return ::Engine::Physics::Update(e, a); }
        BroadPhaseView QueryAABB(const AABB& a) noexcept { return ::Engine::Physics::QueryAABB(a); }
        Hit Raycast(const RayQuery& rq) const noexcept { return ::Engine::Physics::Raycast(rq); }
        void Step(float) noexcept {}
    };
}