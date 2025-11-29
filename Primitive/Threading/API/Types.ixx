export module Prm.Threading:Types;
import Lang.Element;
import Lang.Paradigm;

export namespace Prm {
    struct ThreadHandleTag;
    struct MutexHandleTag;
    struct SemaphoreHandleTag;
    struct EventHandleTag;

    using ThreadFunc   = void(*)(void*) noexcept;
    export using ThreadHandle   = StrongAlias<void*, ThreadHandleTag>;
    export using MutexHandle    = StrongAlias<void*, MutexHandleTag>;
    export using SemaphoreHandle= StrongAlias<void*, SemaphoreHandleTag>;
    export using EventHandle    = StrongAlias<void*, EventHandleTag>;
}
