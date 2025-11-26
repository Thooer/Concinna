export module Meta:TypeTag;
import Element;

export template<typename T>
struct TypeTag { using Type = T; };
export template<typename T>
inline constexpr TypeTag<T> TypeTagV{};
