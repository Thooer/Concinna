module;
export module Engine.Physics:Raycast;

import Language;
import :Types;

export namespace Engine::Physics {
  Hit Raycast(const RayQuery&) noexcept;
}