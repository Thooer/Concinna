module;
export module Engine.Physics:BroadPhase;

import Lang;
import :Types;

export namespace Engine::Physics {
  BroadPhaseView QueryAABB(const AABB&) noexcept;
  bool Update(Entity, const AABB&) noexcept;
  bool Remove(Entity) noexcept;
}