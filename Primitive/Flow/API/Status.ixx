export module Prm.Flow:Status;
import Prm.Element;
import :StatusCode;

export namespace Prm {
    struct Status {
        [[no_unique_address]] StatusCode code{};
        const char* message{nullptr};
        [[nodiscard]] constexpr bool Ok() const noexcept { return code.IsSuccess(); }
    };
}

