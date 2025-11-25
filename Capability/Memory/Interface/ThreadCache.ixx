export module Memory:ThreadCache;

import Language;

export namespace Memory {
    struct ThreadLocalContext {
        Atomic<UIntPtr> remoteFreeHead{0};
    };

    export struct ThreadMemory {
        static void Init() noexcept;
        static void Shutdown() noexcept;
        [[nodiscard]] static ThreadLocalContext* Get() noexcept;
    };
}