export module Flow:Functional;
import Element;
import Semantics;
import :Error;
import :Expect;
import :StatusHelpers;

export 
template<typename T> struct IsExpectImpl { static constexpr bool value = false; };
template<typename U> struct IsExpectImpl<Expect<U>> { static constexpr bool value = true; };
template<typename T>
concept IsExpect = IsExpectImpl<typename std::remove_cvref<T>::type>::value;
template<typename ER> struct ExpectValue;
template<typename U> struct ExpectValue<Expect<U>> { using type = U; };
template<typename ER> using ExpectValueT = typename ExpectValue<typename std::remove_cvref<ER>::type>::type;
template<typename T>
struct Chain {
    T m_value{};

    constexpr Chain() noexcept = default;
    constexpr Chain(const T& v) noexcept : m_value(v) {}
    constexpr Chain(T&& v) noexcept : m_value(Move(v)) {}

    template<auto& CPO, typename... Args>
    constexpr auto Call(Args&&... args) noexcept(noexcept(CPO(m_value, Forward<Args>(args)...))) {
        using R = decltype(CPO(m_value, Forward<Args>(args)...));
        if constexpr (SameAs<R, void>) {
            CPO(m_value, Forward<Args>(args)...);
            return Chain<T>{ Move(m_value) };
        } else {
            return Chain<R>{ CPO(m_value, Forward<Args>(args)...) };
        }
    }

    template<auto& CPO, typename... Args>
    constexpr auto call(Args&&... args) noexcept(noexcept(CPO(m_value, Forward<Args>(args)...))) {
        return Call<CPO>(Forward<Args>(args)...);
    }

    template<typename F>
    constexpr auto Map(F&& f) noexcept(noexcept(f(m_value))) {
        if constexpr (IsExpect<T>) {
            using Ok = ExpectValueT<T>;
            using R = std::conditional_t<SameAs<Ok, void>, decltype(f()), decltype(f(std::declval<Ok&>()))>;
            if constexpr (IsExpect<R>) {
                if constexpr (SameAs<Ok, void>) {
                    auto r = (Move(m_value) | AndThen([&](){ return f(); }));
                    return Chain<R>{ Move(r) };
                } else {
                    auto r = (Move(m_value) | AndThen([&](Ok& v){ return f(v); }));
                    return Chain<R>{ Move(r) };
                }
            } else {
                if constexpr (SameAs<Ok, void>) {
                    auto r = (Move(m_value) | Map([&](){ return f(); }));
                    return Chain<Expect<R>>{ Move(r) };
                } else {
                    auto r = (Move(m_value) | Map([&](Ok& v){ return f(v); }));
                    return Chain<Expect<R>>{ Move(r) };
                }
            }
        } else {
            using R = decltype(f(m_value));
            if constexpr (SameAs<R, void>) {
                f(m_value);
                return Chain<T>{ Move(m_value) };
            } else {
                return Chain<R>{ f(m_value) };
            }
        }
    }

    template<typename F, typename... Args>
    constexpr auto Apply(F&& f, Args&&... args) noexcept(noexcept(f(m_value, Forward<Args>(args)...))) {
        if constexpr (IsExpect<T>) {
            using Ok = ExpectValueT<T>;
            using R = std::conditional_t<SameAs<Ok, void>, decltype(f(Forward<Args>(args)...)), decltype(f(std::declval<Ok&>(), Forward<Args>(args)...))>;
            if constexpr (IsExpect<R>) {
                if constexpr (SameAs<Ok, void>) {
                    auto r = (Move(m_value) | AndThen([&](){ return f(Forward<Args>(args)...); }));
                    return Chain<R>{ Move(r) };
                } else {
                    auto r = (Move(m_value) | AndThen([&](Ok& v){ return f(v, Forward<Args>(args)...); }));
                    return Chain<R>{ Move(r) };
                }
            } else {
                if constexpr (SameAs<Ok, void>) {
                    auto r = (Move(m_value) | Map([&](){ return f(Forward<Args>(args)...); }));
                    return Chain<Expect<R>>{ Move(r) };
                } else {
                    auto r = (Move(m_value) | Map([&](Ok& v){ return f(v, Forward<Args>(args)...); }));
                    return Chain<Expect<R>>{ Move(r) };
                }
            }
        } else {
            using R = decltype(f(m_value, Forward<Args>(args)...));
            if constexpr (SameAs<R, void>) {
                f(m_value, Forward<Args>(args)...);
                return Chain<T>{ Move(m_value) };
            } else {
                return Chain<R>{ f(m_value, Forward<Args>(args)...) };
            }
        }
    }

    [[nodiscard]] constexpr T& ValueRef() & noexcept { return m_value; }
    [[nodiscard]] constexpr const T& ValueRef() const & noexcept { return m_value; }
    [[nodiscard]] constexpr T&& ValueRef() && noexcept { return Move(m_value); }
};

export template<typename T>
Chain(T) -> Chain<T>;

export template<typename T, typename Op>
[[nodiscard]] constexpr auto operator|(T&& v, Op&& op) noexcept(noexcept(op(static_cast<T&&>(v)))) -> decltype(op(static_cast<T&&>(v))) {
    return op(static_cast<T&&>(v));
}

export template<typename T, typename Op>
[[nodiscard]] constexpr auto operator|(Chain<T>&& c, Op&& op) noexcept(noexcept(op(c.ValueRef()))) -> decltype(op(c.ValueRef())) {
    return op(c.ValueRef());
}
export struct MapOp {
    template<typename F>
    struct Impl {
        Decay<F> fn;
        template<typename X>
        [[nodiscard]] constexpr auto operator()(X&& x) const {
            if constexpr (IsExpect<X>) {
                using Ok = ExpectValueT<X>;
                using R = std::conditional_t<SameAs<Ok, void>, decltype(fn()), decltype(fn(std::declval<Ok&>()))>;
                if (x.IsOk()) {
                    if constexpr (IsExpect<R>) {
                        if constexpr (SameAs<Ok, void>) { return fn(); } else { return fn(x.Value()); }
                    } else {
                        if constexpr (SameAs<Ok, void>) { return Expect<RemoveCvRef<R>>::Ok(fn()); }
                        else { return Expect<RemoveCvRef<R>>::Ok(fn(x.Value())); }
                    }
                } else {
                    if constexpr (IsExpect<R>) {
                        return R::Err(Move(x.Error()));
                    } else {
                        return Expect<RemoveCvRef<R>>::Err(Move(x.Error()));
                    }
                }
            } else {
                return fn(static_cast<X&&>(x));
            }
        }
    };
    template<typename F>
    [[nodiscard]] constexpr auto operator()(F&& f) const { return Impl<F>{ Forward<F>(f) }; }
};
export inline constexpr MapOp Map{};

export struct AndThenOp {
    template<typename F>
    struct Impl {
        Decay<F> fn;
        template<typename X>
        [[nodiscard]] constexpr auto operator()(X&& x) const {
            if constexpr (IsExpect<X>) {
                using Ok = ExpectValueT<X>;
                if (x.IsOk()) { if constexpr (SameAs<Ok, void>) { return fn(); } else { return fn(x.Value()); } }
                else { return Expect<Ok>::Err(Move(x.Error())); }
            } else { return fn(static_cast<X&&>(x)); }
        }
    };
    template<typename F>
    [[nodiscard]] constexpr auto operator()(F&& f) const { return Impl<F>{ Forward<F>(f) }; }
};
export inline constexpr AndThenOp AndThen{};

export struct OrElseOp {
    template<typename G>
    struct Impl {
        Decay<G> fn;
        template<typename X>
        [[nodiscard]] constexpr auto operator()(X&& x) const {
            if constexpr (IsExpect<X>) { if (x.IsOk()) { return static_cast<X&&>(x); } else { return fn(x.Error()); } }
            else { return fn(static_cast<X&&>(x)); }
        }
    };
    template<typename G>
    [[nodiscard]] constexpr auto operator()(G&& g) const { return Impl<G>{ Forward<G>(g) }; }
};
export inline constexpr OrElseOp OrElse{};

export template<typename R>
[[nodiscard]] inline Expect<void> Try(R&& r) noexcept {
    auto tmp = static_cast<R&&>(r);
    if (!tmp.IsOk()) {
        return Expect<void>::Err(Move(tmp.Error()));
    }
    return Expect<void>::Ok(Ok(StatusDomain::Generic()));
}

export template<typename R>
[[nodiscard]] inline Expect<void> TryAssign(typename ExpectValue<R>::type& var, R&& r) noexcept {
    auto tmp = static_cast<R&&>(r);
    if (!tmp.IsOk()) {
        return Expect<void>::Err(Move(tmp.Error()));
    }
    var = Move(tmp.Value());
    return Expect<void>::Ok(Ok(StatusDomain::Generic()));
}
