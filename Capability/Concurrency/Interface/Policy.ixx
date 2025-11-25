export module Concurrency:Policy;

import Language;
import Memory;
import Platform;

export namespace Concurrency::Policy {
    struct Ebr {
        using Manager = Memory::EbrManager;
        using Record = Memory::EbrRecord*;
        static Memory::EbrRecord* Register(Manager& m) noexcept { return m.Register(); }
        static void Enter(Manager& m, Memory::EbrRecord* r) noexcept { m.Enter(r); }
        static void Exit(Manager& m, Memory::EbrRecord* r) noexcept { m.Exit(r); }
        static void Advance(Manager& m) noexcept { m.Advance(); }
        static void Collect(Manager& m) noexcept { m.Collect(); }
        static void Retire(Manager& m, void* p, void* ctx, void(*f)(void*, void*) noexcept) noexcept { m.Retire(p, ctx, f); }
    };

    struct None {
        struct Manager {};
        using Record = void*;
        static Record Register(Manager&) noexcept { return nullptr; }
        static void Enter(Manager&, void*) noexcept {}
        static void Exit(Manager&, void*) noexcept {}
        static void Advance(Manager&) noexcept {}
        static void Collect(Manager&) noexcept {}
        static void Retire(Manager&, void* p, void* ctx, void(*f)(void*, void*) noexcept) noexcept { f(p, ctx); }
    };

    struct Spin { static void Wait(Backoff& b) noexcept { b.Next(); } };
    struct Yield { static void Wait(Backoff&) noexcept { Platform::ThreadYield(); } };
}