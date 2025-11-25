export module Prm.Flow:StatusCode;
import Prm.Element;
import :StatusDomain;

export namespace Prm {
    struct StatusCode {
        using Value = StatusCodeValue;
        enum : Value { Ok = 0, Failed = 1, InvalidArgument = 2, OutOfRange = 3, Timeout = 4, Unsupported = 5, NotFound = 6 };
        [[no_unique_address]] StatusDomain domain{};
        StatusCodeValue value{StatusCode::Ok};
        [[nodiscard]] constexpr bool IsSuccess() const noexcept { return value == domain.success; }
        [[nodiscard]] constexpr bool IsUnknown() const noexcept { return value == domain.unknown; }
    };
}

