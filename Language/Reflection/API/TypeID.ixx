export module Lang.Reflection:TypeID;
import Lang.Element;
import <concepts>;

export struct TypeID {
    UInt64 value{};
    [[nodiscard]] constexpr bool operator==(const TypeID& rhs) const noexcept { return value == rhs.value; }
    [[nodiscard]] constexpr bool operator!=(const TypeID& rhs) const noexcept { return value != rhs.value; }
};

export constexpr UInt64 SimpleStringHash(const char* s) noexcept {
    UInt64 hash = 5381;
    for (const char* p = s; *p; ++p) { hash = ((hash << 5) + hash) + static_cast<UInt64>(*p); }
    return hash;
}
export struct SignatureHasherDefault {
    static consteval UInt64 Hash(const char* s) noexcept { return SimpleStringHash(s); }
};

export template<typename T, typename Hasher = SignatureHasherDefault>
consteval TypeID GetTypeID() noexcept {
    if constexpr (requires { { T::StableName() } -> std::same_as<const char*>; }) {
        return TypeID{ Hasher::Hash(T::StableName()) };
    } else {
        static_assert(sizeof(T) == 0, "Provide stable type name via T::StableName().");
    }
}
