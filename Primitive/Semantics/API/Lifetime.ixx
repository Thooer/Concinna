module;

export module Prm.Semantics:Lifetime;
import Prm.Element;
import <cstring>;
import <new>;

// Core 模块禁止直接引入标准库，请避免 std 依赖

export namespace Prm {
    export inline void UnsafeScopeAnnotate(const char*) noexcept {}
    // Borrow —— 非拥有借用包装器：仅持有指针，不管理生命周期
    template<typename T>
    struct Borrow {
        using Element = T;

        T* m_ptr{};

        constexpr Borrow() noexcept = default;
        constexpr explicit Borrow(T* p) noexcept : m_ptr(p) {}

        [[nodiscard]] constexpr T* Get() const noexcept { return m_ptr; }
        [[nodiscard]] constexpr bool Has() const noexcept { return m_ptr != nullptr; }
        [[nodiscard]] constexpr T& Ref() const noexcept { ASSERT(m_ptr != nullptr, "Borrow: null"); return *m_ptr; }

        constexpr explicit operator bool() const noexcept { return m_ptr != nullptr; }
    };

    // 便捷工厂：从引用或指针创建 Borrow
    template<typename T>
    [[nodiscard]] constexpr Borrow<T> MakeBorrow(T& r) noexcept { return Borrow<T>(&r); }
    template<typename T>
    [[nodiscard]] constexpr Borrow<T> MakeBorrow(T* p) noexcept { return Borrow<T>(p); }



    namespace detail {
        template<typename F>
        struct CallDeleter {
            constexpr void operator()(F& f) const noexcept { f(); }
        };
    }


    template<typename T>
    struct Out {
        T* m_ptr{};
        constexpr Out() noexcept = default;
        constexpr explicit Out(T& r) noexcept : m_ptr(&r) {}
        constexpr explicit Out(T* p) noexcept : m_ptr(p) {}
        [[nodiscard]] constexpr bool Has() const noexcept { return m_ptr != nullptr; }
        [[nodiscard]] constexpr T& Ref() const noexcept { ASSERT(m_ptr != nullptr, "Out: null"); return *m_ptr; }
        constexpr void Set(const T& v) const noexcept(noexcept(*m_ptr = v)) { ASSERT(m_ptr != nullptr, "Out: null"); *m_ptr = v; }
        constexpr void Set(T&& v) const noexcept(noexcept(*m_ptr = Move(v))) { ASSERT(m_ptr != nullptr, "Out: null"); *m_ptr = Move(v); }
    };

    template<typename T>
    struct Transfer {
        T m_value{};
        constexpr Transfer() = delete;
        constexpr explicit Transfer(T&& v) noexcept(NothrowMoveConstructible<T>) : m_value(Move(v)) {}
        Transfer(const Transfer&) = delete;
        Transfer& operator=(const Transfer&) = delete;
        constexpr Transfer(Transfer&&) noexcept = default;
        constexpr Transfer& operator=(Transfer&&) noexcept = default;
        [[nodiscard]] constexpr const T& Peek() const& noexcept { return m_value; }
        [[nodiscard]] constexpr T Take() noexcept(NothrowMoveConstructible<T>) { return Move(m_value); }
    };

    template<typename T>
    [[nodiscard]] constexpr Transfer<T> MakeTransfer(T&& v) noexcept(NothrowMoveConstructible<T>) {
        return Transfer<T>{ Move(v) };
    }



    using MaxAlignT = std::max_align_t;
    using AlignValT = std::align_val_t;


    template<typename F>
    struct Defer {
        F f;
        ~Defer() noexcept(noexcept(f())) { f(); }
    };
    template<typename F>
    inline auto MakeDefer(F&& f) noexcept(noexcept(F(static_cast<F&&>(f)))) -> Defer<F> {
        return Defer<F>{static_cast<F&&>(f)};
    }





        // 要求删除器在调用上为 noexcept（复用 Prm 概念）
    template<typename Deleter, typename Credential>
    concept NoexceptDeleter = NothrowCallable<Deleter, Credential>;

    // UniqueResource: 将“所有权”与“资源凭证”解耦的通用 RAII 管理器
    // 约束：Deleter(credential) 必须 noexcept
    template<typename Credential, typename Deleter>
    requires NoexceptDeleter<Deleter, Credential>
    struct UniqueResource {
        using CredentialType = Credential;
        using DeleterType = Deleter;

        constexpr UniqueResource() noexcept = default;

        constexpr UniqueResource(Credential cred, Deleter deleter) noexcept(
            NothrowMoveConstructible<Credential> && NothrowMoveConstructible<Deleter>)
            : m_credential(Move(cred)), m_deleter(Move(deleter)), m_engaged(true) {}

        UniqueResource(const UniqueResource&) = delete;
        UniqueResource& operator=(const UniqueResource&) = delete;

        constexpr UniqueResource(UniqueResource&& other) noexcept(
            NothrowMoveConstructible<Credential> && NothrowMoveConstructible<Deleter>)
            : m_credential(Move(other.m_credential)),
              m_deleter(Move(other.m_deleter)),
              m_engaged(other.m_engaged) {
            other.m_engaged = false;
        }

        constexpr UniqueResource& operator=(UniqueResource&& other) noexcept(
            NothrowMoveConstructible<Credential> && IsNothrowAssignable<Deleter, Deleter&&>)
        {
            if (this == &other) return *this;
            Reset();
            m_credential = Move(other.m_credential);
            m_deleter = Move(other.m_deleter);
            m_engaged = other.m_engaged;
            other.m_engaged = false;
            return *this;
        }

        ~UniqueResource() noexcept { Reset(); }

        [[nodiscard]] constexpr bool Has() const noexcept { return m_engaged; }

        [[nodiscard]] constexpr Credential& Get() & noexcept { return m_credential; }
        [[nodiscard]] constexpr const Credential& Get() const& noexcept { return m_credential; }
        [[nodiscard]] constexpr Credential&& Take() && noexcept { return Move(m_credential);
        }

        // 释放所有权，返回凭证，并且不触发删除器
        [[nodiscard]] constexpr Credential Release() noexcept {
            m_engaged = false;
            return Move(m_credential);
        }

        // 重置资源：若持有则执行删除器；然后置空为未持有
        constexpr void Reset() noexcept {
            if (m_engaged) {
                m_deleter(m_credential);
                m_engaged = false;
            }
        }

        // 替换资源：先删除已持有资源，然后接管新凭证
        constexpr void Reset(Credential cred) noexcept(IsNothrowAssignable<Credential, Credential&&>) {
            Reset();
            m_credential = Move(cred);
            m_engaged = true;
        }

        // 交换两者的资源与删除器
        constexpr void Swap(UniqueResource& other) noexcept(
            NothrowMoveConstructible<Credential> && IsNothrowAssignable<Credential, Credential&&> &&
            NothrowMoveConstructible<Deleter> && IsNothrowAssignable<Deleter, Deleter&&>)
        {
            auto tmpCred = Move(m_credential);
            m_credential = Move(other.m_credential);
            other.m_credential = Move(tmpCred);

            auto tmpDel = Move(m_deleter);
            m_deleter = Move(other.m_deleter);
            other.m_deleter = Move(tmpDel);

            const bool tmpE = m_engaged;
            m_engaged = other.m_engaged;
            other.m_engaged = tmpE;
        }

    private:
        Credential m_credential{};
        [[no_unique_address]] Deleter m_deleter{};
        bool m_engaged{false};
    };

    template<typename Policy>
    using UniqueHandle = UniqueResource<typename Policy::Credential, Policy>;

    // Scoped —— 唯一所有权：以指针作为凭证的 RAII 管理别名
    // 注意：删除器必须满足 NoexceptDeleter<Deleter, T*> 约束
    template<typename T, typename Deleter>
    using Scoped = UniqueResource<T*, Deleter>;



    // ScopeGuard：作用域结束时执行回调；可取消
    template<typename F>
    struct ScopeGuard {
        static_assert(NothrowCallable<F>, "ScopeGuard requires a noexcept callable");
        using Guard = UniqueResource<F, detail::CallDeleter<F>>;

        Guard m_guard{};

        constexpr ScopeGuard() noexcept = default;
        constexpr explicit ScopeGuard(F&& f) noexcept(noexcept(Guard(Forward<F>(f), detail::CallDeleter<F>{})))
            : m_guard(Forward<F>(f), detail::CallDeleter<F>{}) {}

        // 取消执行（释放所有权，不触发删除器）
        constexpr void Cancel() noexcept { (void)m_guard.Release(); }
    };
    inline void* MemMove(void* dst, const void* src, USize n) noexcept {
        if (n == 0 || dst == src) return dst;
        return std::memmove(dst, src, static_cast<std::size_t>(n));
    }

    template<typename T, typename... Args>
    inline T* ConstructAt(void* p, Args&&... args) noexcept(noexcept(T(Forward<Args>(args)...))) {
        return ::new (p) T(Forward<Args>(args)...);
    }

    template<typename T>
    inline void DestroyAt(T* p) noexcept {
        if constexpr (!TriviallyDestructible<T>) { p->~T(); }
    }
}
