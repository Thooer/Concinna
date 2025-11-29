export module Lang.Meta:CallableTraits;
import Lang.Element;
import <tuple>;
import <type_traits>;

// 显式导出主模板声明
export template<typename T>
struct CallableTraits;

// 以下特化会自动随主模板可见，无需每个都加 export

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

// 主模板定义（用于仿函数/Lambda）
template<typename F>
struct CallableTraits : CallableTraits<decltype(&std::remove_cvref_t<F>::operator())> {};
