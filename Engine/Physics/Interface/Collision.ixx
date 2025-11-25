module;
export module Engine.Physics:Collision;

import Language;
import :Types;
import Foundation.Containers;
// use Collider from :Types

export namespace Engine::Physics {
  bool AddCollider(Entity, const ColliderAABB&) noexcept;
  bool RemoveCollider(Entity) noexcept;
  export Foundation::Containers::Vector<Collider>& GetColliders() noexcept;
}