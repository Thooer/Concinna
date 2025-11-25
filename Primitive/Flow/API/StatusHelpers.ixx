export module Prm.Flow:StatusHelpers;
import Prm.Element;
import :StatusDomain;
import :StatusCode;
import :Status;

export namespace Prm {
    [[nodiscard]] inline constexpr Status Ok(StatusDomain domain) noexcept { return Status{ StatusCode{ domain, StatusCode::Ok }, nullptr }; }
    [[nodiscard]] inline constexpr Status Err(StatusDomain domain, StatusCodeValue value, const char* msg = nullptr) noexcept { return Status{ StatusCode{ domain, value }, msg }; }
}

