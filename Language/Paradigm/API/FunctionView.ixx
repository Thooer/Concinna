module;

export module Paradigm:FunctionView;

import Element;
import Semantics;


 

    // 基本版 FunctionView：R(Args...) 调用约定
    export template<typename> struct FunctionView;

    export template<typename R, typename... Args>
    struct FunctionView<R(Args...)> {
        using Result = R;
        using Invoker = R(*)(void*, Args&&...);

        constexpr FunctionView() noexcept = default;

        template<typename F>
        requires Invocable<F&, Args...> && ConvertibleTo<InvokeResult<F, Args...>, R>
        constexpr FunctionView(F& f) noexcept { Bind(f); }

        template<typename F>
        constexpr FunctionView(F&&) = delete;

        template<typename F>
        requires Invocable<F&, Args...> && ConvertibleTo<InvokeResult<F, Args...>, R>
        constexpr void Bind(F& f) noexcept {
            // 非 P 模块禁止使用 std::addressof；此处采用 &f
            m_object = static_cast<void*>(&f);
            m_call = +[](void* p, Args&&... as) -> R {
                auto* fp = static_cast<F*>(p);
                if constexpr (SameAs<R, void>) {
                    (*fp)(Forward<Args>(as)...);
                } else {
                    return (*fp)(Forward<Args>(as)...);
                }
            };
        }

        [[nodiscard]] constexpr bool HasTarget() const noexcept { return m_call != nullptr; }
        [[nodiscard]] explicit constexpr operator bool() const noexcept { return HasTarget(); }

        constexpr void Reset() noexcept { m_object = nullptr; m_call = nullptr; }

        constexpr R operator()(Args... args) const {
            ASSERT(m_call != nullptr, "FunctionView: empty target");
            if constexpr (SameAs<R, void>) {
                m_call(m_object, Forward<Args>(args)...);
            } else {
                return m_call(m_object, Forward<Args>(args)...);
            }
        }

    private:
        void* m_object{nullptr};
        Invoker m_call{nullptr};
    };

    // noexcept 变体：R(Args...) noexcept —— 仅接受不抛异常的可调用对象
    export template<typename R, typename... Args>
    struct FunctionView<R(Args...) noexcept> {
        using Result = R;
        using Invoker = R(*)(void*, Args&&...) noexcept;

        constexpr FunctionView() noexcept = default;

        template<typename F>
        requires Invocable<F&, Args...> && ConvertibleTo<InvokeResult<F, Args...>, R> && NothrowCallable<F, Args...>
        constexpr FunctionView(F& f) noexcept { Bind(f); }

        template<typename F>
        constexpr FunctionView(F&&) = delete;

        template<typename F>
        requires Invocable<F&, Args...> && ConvertibleTo<InvokeResult<F, Args...>, R> && NothrowCallable<F, Args...>
        constexpr void Bind(F& f) noexcept {
            m_object = static_cast<void*>(&f);
            m_call = +[](void* p, Args&&... as) noexcept -> R {
                auto* fp = static_cast<F*>(p);
                if constexpr (SameAs<R, void>) {
                    (*fp)(Forward<Args>(as)...);
                } else {
                    return (*fp)(Forward<Args>(as)...);
                }
            };
        }

        [[nodiscard]] constexpr bool HasTarget() const noexcept { return m_call != nullptr; }
        [[nodiscard]] explicit constexpr operator bool() const noexcept { return HasTarget(); }

        constexpr void Reset() noexcept { m_object = nullptr; m_call = nullptr; }

        constexpr R operator()(Args... args) const noexcept {
            ASSERT(m_call != nullptr, "FunctionView(noexcept): empty target");
            if constexpr (SameAs<R, void>) {
                m_call(m_object, Forward<Args>(args)...);
            } else {
                return m_call(m_object, Forward<Args>(args)...);
            }
        }

    private:
        void* m_object{nullptr};
        Invoker m_call{nullptr};
    };
    
