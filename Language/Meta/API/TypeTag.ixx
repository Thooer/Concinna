export module Lang.Meta:TypeTag;
import Lang.Element;

export template<typename T>
struct TypeTag { using Type = T; };
export template<typename T>
inline constexpr TypeTag<T> TypeTagV{};
