export module Prm.Meta:Conversion;
import Prm.Element;

export namespace Prm {
    template<Enum E>
    using UnderlyingTypeT = std::underlying_type_t<E>;
    template<Enum E>
    [[nodiscard]] constexpr E FromUnderlying(UnderlyingTypeT<E> v) noexcept { return static_cast<E>(v); }
}

