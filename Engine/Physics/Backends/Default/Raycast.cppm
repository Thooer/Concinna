module;
export module Engine.Physics:Raycast;

import Lang;
import :Types;
import :DBVT;

namespace Engine::Physics {
  export Hit Raycast(const RayQuery& rq) noexcept { return DBVT_Raycast(rq); }
}