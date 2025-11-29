module;
export module Engine.Physics.DBVTPerfSmoke;

import Lang;
import Engine.Physics;
import Foundation.Time;

namespace Nova::Samples::PhysicsDBVTPerf {
  export bool Run() noexcept {
    using namespace Engine::Physics;
    Foundation::Time::SteadyClock::Init();
    const Language::UInt32 N = 10000;
    for (Language::UInt32 i = 0; i < N; ++i) {
      Language::UInt32 gx = i % 100;
      Language::UInt32 gy = (i / 100) % 100;
      Vec3 p{ static_cast<Language::Float32>(gx), static_cast<Language::Float32>(gy), 0.0f };
      AABB a{ { p.x, p.y, 0.0f }, { p.x + 0.5f, p.y + 0.5f, 1.0f } };
      (void)AddCollider(i + 1, a);
    }
    double sumQueryMs = 0.0;
    double sumRayMs = 0.0;
    for (Language::UInt32 k = 0; k < 100; ++k) {
      AABB q{ { static_cast<Language::Float32>(k), static_cast<Language::Float32>(k), 0.0f }, { static_cast<Language::Float32>(k + 50), static_cast<Language::Float32>(k + 50), 1.0f } };
      auto t0 = Foundation::Time::SteadyClock::Now();
      (void)QueryAABB(q);
      auto t1 = Foundation::Time::SteadyClock::Now();
      sumQueryMs += Foundation::Time::SteadyClock::ToMilliseconds(Foundation::Time::SteadyClock::Delta(t0, t1));
      RayQuery rq{}; rq.origin = { -10.0f, static_cast<Language::Float32>(k + 0.25f), 0.5f }; rq.dir = { 200.0f, 0.0f, 0.0f };
      t0 = Foundation::Time::SteadyClock::Now();
      (void)Raycast(rq);
      t1 = Foundation::Time::SteadyClock::Now();
      sumRayMs += Foundation::Time::SteadyClock::ToMilliseconds(Foundation::Time::SteadyClock::Delta(t0, t1));
    }
    return true;
  }
}