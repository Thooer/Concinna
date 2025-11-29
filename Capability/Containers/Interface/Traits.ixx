export module Containers:Traits;

import Lang;
import :String;
import <type_traits>;

export namespace Containers {
    template<typename T>
    struct IsTriviallyRelocatable : std::bool_constant<TriviallyCopyable<T>> {};

    template<typename T>
    concept TriviallyRelocatable = IsTriviallyRelocatable<T>::value;

    template<typename T>
    struct IsTriviallyComparable : std::bool_constant<std::has_unique_object_representations_v<T> && std::is_trivially_copyable_v<T>> {};

    template<typename T>
    concept TriviallyComparable = IsTriviallyComparable<T>::value;

    template<typename T>
    struct IsZeroConstructible : std::bool_constant<std::is_trivially_default_constructible_v<T> && std::is_trivially_copyable_v<T>> {};

    template<typename T>
    concept ZeroConstructible = IsZeroConstructible<T>::value;

    template<typename T>
    struct AllocatorAware : std::false_type {};

    template<typename P>
    struct AllocatorAware<::Containers::String<P>> : std::true_type {};

    template<typename T>
    concept IsAllocatorAware = AllocatorAware<T>::value;
}