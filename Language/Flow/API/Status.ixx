export module Lang.Flow:Status;
import Lang.Element;
import :StatusCode;
export struct Status {
    [[no_unique_address]] StatusCode code{};
    const char* message{nullptr};
    [[nodiscard]] constexpr bool Ok() const noexcept { return code.IsSuccess(); }
};
