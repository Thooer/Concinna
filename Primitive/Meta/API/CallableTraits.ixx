export module Prm.Meta:CallableTraits;
import Prm.Element;
import <tuple>;
import <type_traits>;

export namespace Prm {
    template<typename T>
    struct CallableTraits;
    template<typename R, typename... Args>
    struct CallableTraits<R(Args...)> { using Return = R; using ArgsTuple = std::tuple<Args...>; static constexpr USize Arity = sizeof...(Args); };
    template<typename R, typename... Args>
    struct CallableTraits<R(Args...) noexcept> : CallableTraits<R(Args...)> {};
    template<typename R, typename... Args>
    struct CallableTraits<R(*)(Args...)> : CallableTraits<R(Args...)> {};
    template<typename R, typename... Args>
    struct CallableTraits<R(*)(Args...) noexcept> : CallableTraits<R(Args...)> {};
    template<typename R, typename... Args>
    struct CallableTraits<R(&)(Args...)> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...)> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) const> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) &> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) const&> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) noexcept> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) const noexcept> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) volatile> : CallableTraits<R(Args...)> {};
    template<typename R, typename C, typename... Args>
    struct CallableTraits<R(C::*)(Args...) const volatile> : CallableTraits<R(Args...)> {};
    template<typename F>
    struct CallableTraits : CallableTraits<decltype(&std::remove_cvref_t<F>::operator())> {};
}

