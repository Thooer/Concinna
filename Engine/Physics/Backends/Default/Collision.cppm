module;
export module Engine.Physics:Collision;

import Lang;
import :Types;
import Foundation.Memory;
// use Collider from :Types
import :DBVT;

namespace Engine::Physics {
  static Foundation::Containers::Vector<Collider>& colliders() noexcept {
    static ::Foundation::Memory::IAllocator* a = []() noexcept {
      auto r = ::Foundation::Memory::CreateDefaultAllocator();
      return r.IsOk() ? r.OkValue() : nullptr;
    }();
    static Foundation::Containers::Vector<Collider> v(a);
    return v;
  }

  export Foundation::Containers::Vector<Collider>& GetColliders() noexcept { return colliders(); }

  export bool AddCollider(Entity e, const ColliderAABB& a) noexcept {
    auto& g = colliders();
    for (Language::USize i = 0; i < g.size(); ++i) {
      if (g.data()[static_cast<size_t>(i)].entity == e) return false;
    }
    Collider c{}; c.entity = e; c.bounds = a;
    bool ok = DBVT_Insert(e, a);
    if (!ok) return false;
    return g.push_back(c);
  }

  export bool RemoveCollider(Entity e) noexcept {
    auto& g = colliders();
    for (Language::USize i = 0; i < g.size(); ++i) {
      if (g.data()[static_cast<size_t>(i)].entity == e) {
        (void)DBVT_Remove(e);
        g.data()[static_cast<size_t>(i)] = g.data()[static_cast<size_t>(g.size() - 1)];
        (void)g.resize(g.size() - static_cast<Language::USize>(1));
        return true;
      }
    }
    return false;
  }
}