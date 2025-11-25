export module Prm.Flow:StatusDomain;
import Prm.Element;

export namespace Prm {
    using StatusCodeValue = Int32;
    struct StatusDomain {
        const char* name;
        StatusCodeValue success;
        StatusCodeValue unknown;
        [[nodiscard]] static constexpr StatusDomain Generic() noexcept { return StatusDomain{"Generic", 0, -1}; }
        [[nodiscard]] static constexpr StatusDomain System() noexcept { return StatusDomain{"System", 0, -1}; }
    };
}

