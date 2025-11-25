export module Prm.Meta:TypeTag;
import Prm.Element;

export namespace Prm {
    template<typename T>
    struct TypeTag { using Type = T; };
    template<typename T>
    inline constexpr TypeTag<T> TypeTagV{};
}

