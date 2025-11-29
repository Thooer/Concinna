module;
#include <new>
#include <utility>

export module Lang.Flow:Expect;
import Lang.Element;
import Lang.Semantics;
import :Status;


export template<typename T>
struct [[nodiscard("Result must be checked")]] Expect {
    bool hasValue{false};
    union U { T value; Status status; U() {} ~U() {} } u{};

    Expect() noexcept : hasValue(false) { new (&u.status) Status{}; }
    Expect(const Expect&) = delete;
    Expect& operator=(const Expect&) = delete;
    Expect(Expect&& other) noexcept {
        hasValue = other.hasValue;
        if (hasValue) {
            if constexpr (TriviallyCopyable<T>) new (&u.value) T(other.u.value);
            else new (&u.value) T(std::move(other.u.value));
            if constexpr (!TriviallyDestructible<T>) other.u.value.~T();
            other.hasValue = false;
            new (&other.u.status) Status{};
        } else {
            new (&u.status) Status(std::move(other.u.status));
            if constexpr (!TriviallyDestructible<Status>) other.u.status.~Status();
            new (&other.u.status) Status{};
        }
    }

    Expect& operator=(Expect&& other) noexcept {
        if (this == &other) return *this;
        if (hasValue) { u.value.~T(); }
        hasValue = other.hasValue;
        if (hasValue) {
            if constexpr (TriviallyCopyable<T>) new (&u.value) T(other.u.value);
            else new (&u.value) T(std::move(other.u.value));
            if constexpr (!TriviallyDestructible<T>) other.u.value.~T();
            other.hasValue = false;
            new (&other.u.status) Status{};
        } else {
            new (&u.status) Status(std::move(other.u.status));
            if constexpr (!TriviallyDestructible<Status>) other.u.status.~Status();
            new (&other.u.status) Status{};
        }
        return *this;
    }

    ~Expect() noexcept { if (hasValue) { u.value.~T(); } }

    [[nodiscard]] inline bool IsOk() const noexcept { return hasValue; }
    [[nodiscard]] inline bool Ok() const noexcept { return IsOk(); }
    [[nodiscard]] inline T&       Value() & noexcept { return u.value; }
    [[nodiscard]] inline const T& Value() const& noexcept { return u.value; }
    [[nodiscard]] inline T&&      Value() && noexcept { return Move(u.value); }
    [[nodiscard]] inline Status&       Error() & noexcept { return u.status; }
    [[nodiscard]] inline const Status& Error() const& noexcept { return u.status; }

    template<typename... Args>
    static inline Expect Ok(Args&&... args) noexcept {
        Expect r; r.hasValue = true; new (&r.u.value) T(std::forward<Args>(args)...); return r;
    }
    template<typename... Args>
    static inline Expect Err(Args&&... args) noexcept {
        Expect r; r.hasValue = false; new (&r.u.status) Status(std::forward<Args>(args)...); return r;
    }
};

export template<>
struct [[nodiscard("Result must be checked")]] Expect<void> {
    Status status{};
    Expect() noexcept = default;
    Expect(Expect&& other) noexcept { status = other.status; }
    Expect& operator=(Expect&& other) noexcept { if (this!=&other){ status=other.status; } return *this; }
    [[nodiscard]] inline bool IsOk() const noexcept { return status.Ok(); }
    [[nodiscard]] inline bool Ok() const noexcept { return IsOk(); }
    [[nodiscard]] inline Status&       Error() & noexcept { return status; }
    [[nodiscard]] inline const Status& Error() const& noexcept { return status; }
    static inline Expect Ok(Status s) noexcept { Expect r; r.status = s; return r; }
    static inline Expect Err(Status s) noexcept { Expect r; r.status = s; return r; }
};
