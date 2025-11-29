module;
export module Engine.Physics:Raycast;

import Lang;
import :Types;

export namespace Engine::Physics {
  Hit Raycast(const RayQuery&) noexcept;
}