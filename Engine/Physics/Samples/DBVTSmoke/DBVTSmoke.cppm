module;
export module Engine.Physics.DBVTSmoke;

import Lang;
import Engine.Physics;

namespace Nova::Samples::PhysicsDBVT {
  static inline bool aabbIntersect(const Engine::Physics::AABB& a, const Engine::Physics::AABB& b) noexcept {
    if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
    if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
    if (a.max.z < b.min.z || a.min.z > b.max.z) return false;
    return true;
  }

  static inline bool rayAabb(const Engine::Physics::AABB& box, const Engine::Physics::Vec3& o, const Engine::Physics::Vec3& d, Float32 maxDist, Float32& outT) noexcept {
    Float32 tmin = 0.0f;
    Float32 tmax = maxDist;
    auto checkAxis = [&](Float32 origin, Float32 dir, Float32 minB, Float32 maxB) noexcept {
      if (dir == 0.0f) { if (origin < minB || origin > maxB) { tmin = 1.0f; tmax = 0.0f; } return; }
      Float32 inv = 1.0f / dir;
      Float32 t1 = (minB - origin) * inv;
      Float32 t2 = (maxB - origin) * inv;
      if (t1 > t2) { Float32 tmp = t1; t1 = t2; t2 = tmp; }
      if (t1 > tmin) tmin = t1;
      if (t2 < tmax) tmax = t2;
    };
    checkAxis(o.x, d.x, box.min.x, box.max.x);
    checkAxis(o.y, d.y, box.min.y, box.max.y);
    checkAxis(o.z, d.z, box.min.z, box.max.z);
    if (tmax >= tmin && tmin <= maxDist && tmax >= 0.0f) { outT = tmin < 0.0f ? tmax : tmin; return true; }
    return false;
  }

  export bool Run() noexcept {
    using namespace Engine::Physics;
    const UInt32 N = 1000;
    for (UInt32 i = 0; i < N; ++i) {
      UInt32 gx = i % 50;
      UInt32 gy = (i / 50) % 20;
      UInt32 gz = (i / (50 * 20)) % 1;
      Vec3 p{ static_cast<Float32>(gx * 2), static_cast<Float32>(gy * 2), static_cast<Float32>(gz * 2) };
      AABB a{ { p.x, p.y, p.z }, { p.x + 1.0f, p.y + 1.0f, p.z + 1.0f } };
      if (!AddCollider(i + 1, a)) return false;
    }

    AABB q{{10.0f, 10.0f, 0.0f}, {30.0f, 30.0f, 2.0f}};
    auto view = QueryAABB(q);
    auto& list = GetColliders();
    USize naive = 0;
    for (USize i = 0; i < list.size(); ++i) if (aabbIntersect(list.data()[static_cast<size_t>(i)].bounds, q)) ++naive;
    if (view.count != naive) return false;

    RayQuery rq{}; rq.origin = { -10.0f, 10.5f, 0.5f }; rq.dir = { 100.0f, 0.0f, 0.0f };
    auto hit = Raycast(rq);
    Float32 maxDist = 100.0f;
    Float32 t = 0.0f;
    Float32 bestT = maxDist;
    Engine::Physics::Entity bestE = 0;
    for (USize i = 0; i < list.size(); ++i) {
      Float32 tt = 0.0f;
      if (rayAabb(list.data()[static_cast<size_t>(i)].bounds, rq.origin, rq.dir, maxDist, tt)) {
        if (tt < bestT) { bestT = tt; bestE = list.data()[static_cast<size_t>(i)].entity; t = tt; }
      }
    }
    if (!hit.hit) return false;
    if (bestE != hit.entity) return false;
    if (t > maxDist) return false;
    return true;
  }
}