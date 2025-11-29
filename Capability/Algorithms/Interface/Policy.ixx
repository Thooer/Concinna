export module Cap.Algorithms:Policy;

import Lang;

export namespace Cap {
    struct ExecutionPolicy {
        enum class Mode : UInt8 { Sequential, Parallel } mode;
        void* context{nullptr};
    };
    inline constexpr ExecutionPolicy Seq{ ExecutionPolicy::Mode::Sequential, nullptr };
    inline constexpr ExecutionPolicy Par{ ExecutionPolicy::Mode::Parallel, nullptr };
}
