export module Cap.Memory:ThreadCache;

import Lang;
import :Definitions;

export namespace Cap {
    struct ThreadLocalContext {
        Atomic<UIntPtr> remoteFreeHead{0};
    };

    export struct ThreadMemory {
        static void Init() noexcept;
        static void Shutdown() noexcept;
        [[nodiscard]] static ThreadLocalContext* Get() noexcept;
    };
}
