module;
#include <type_traits>

export module Lang.Meta:Conversion;
import Lang.Element;

export template<Enum E>
using UnderlyingTypeT = std::underlying_type_t<E>;
export template<Enum E>
[[nodiscard]] constexpr E FromUnderlying(UnderlyingTypeT<E> v) noexcept { return static_cast<E>(v); }
