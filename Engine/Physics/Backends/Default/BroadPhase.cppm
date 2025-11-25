module;
export module Engine.Physics:BroadPhase;

import Language;
import :Types;
import :DBVT;

namespace Engine::Physics {
  export BroadPhaseView QueryAABB(const AABB& q) noexcept { return DBVT_QueryAABB(q); }
  export bool Update(Entity e, const AABB& a) noexcept { return DBVT_Update(e, a); }
  export bool Remove(Entity e) noexcept { return DBVT_Remove(e); }
}