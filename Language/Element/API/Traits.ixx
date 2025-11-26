export module Element:Traits;
import <type_traits>;
import <utility>;


export template<typename T>
using RemoveCvRef = std::remove_cv_t<std::remove_reference_t<T>>;

export template<typename T>
using Decay = std::decay_t<T>;

export template<typename F, typename... Args>
using InvokeResult = RemoveCvRef<decltype(std::declval<F&>()(std::declval<Args>()...))>;

namespace detail {
    template<typename T, template<typename...> class U>
    struct is_instance_of_impl : std::false_type {};

    template<template<typename...> class U, typename... Args>
    struct is_instance_of_impl<U<Args...>, U> : std::true_type {};
}

export template<typename T, template<typename...> class U>
concept IsInstanceOf = detail::is_instance_of_impl<RemoveCvRef<T>, U>::value;
