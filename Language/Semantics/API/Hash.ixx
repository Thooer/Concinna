export module Semantics:Hash;

 
import <string_view>;
import <string>;
import <ranges>;

import Element;
import Meta;

// ============================================================================
// Hash Algorithms
// ============================================================================

 
    
    export constexpr uint64_t kFnvOffset = 0xcbf29ce484222325ull;
    export constexpr uint64_t kFnvPrime = 0x100000001b3ull;
    
    // FNV-1a hash for byte sequences
    export constexpr uint64_t Fnv1aBytes(const uint8_t* data, size_t size) noexcept {
        uint64_t hash = kFnvOffset;
        for (size_t i = 0; i < size; ++i) {
            hash ^= static_cast<uint64_t>(data[i]);
            hash *= kFnvPrime;
        }
        return hash;
    }
    
    // FNV-1a hash for strings
    export constexpr uint64_t Fnv1aString(std::string_view str) noexcept {
        return Fnv1aBytes(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }
    
    // Golden ratio hash for integer mixing
    export constexpr uint64_t kGoldenRatio64 = 0x9e3779b97f4a7c15ull;
    
    export constexpr uint64_t GoldenRatioHash(uint64_t value) noexcept {
        return value * kGoldenRatio64;
    }
    
    // Combine multiple hash values
    export constexpr uint64_t CombineHashes(uint64_t seed, uint64_t hash) noexcept {
        return seed ^ (hash + kGoldenRatio64 + (seed << 6) + (seed >> 2));
    }
    
    // Bit mixing for pointer hashing
    export constexpr uint64_t BitMixing(uint64_t value) noexcept {
        value ^= value >> 33;
        value *= 0xff51afd7ed558ccdull;
        value ^= value >> 33;
        value *= 0xc4ceb9fe1a85ec53ull;
        value ^= value >> 33;
        return value;
    }
    
    // Simple string hash for lightweight usage
    export constexpr uint64_t SimpleStringHash(std::string_view str) noexcept {
        uint64_t hash = 5381;
        for (char c : str) {
            hash = ((hash << 5) + hash) + static_cast<uint64_t>(c);
        }
        return hash;
    }
    
    // Default hash implementation for fundamental types
    export template<typename T>
    requires std::is_fundamental_v<T>
    constexpr uint64_t tag_invoke(hash_t, T value) noexcept {
        return BitMixing(static_cast<uint64_t>(value));
    }
    
    // Hash for string_view
    export constexpr uint64_t tag_invoke(hash_t, std::string_view str) noexcept {
        return Fnv1aString(str);
    }
    
    // Hash for const char*
    export constexpr uint64_t tag_invoke(hash_t, const char* str) noexcept {
        return Fnv1aString(str);
    }
    
    // Hash for std::string
    constexpr uint64_t tag_invoke(hash_t, const std::string& str) noexcept {
        return Fnv1aString(str);
    }
    
    // Hash for pointers
    export template<typename T>
    constexpr uint64_t tag_invoke(hash_t, T* ptr) noexcept {
        return BitMixing(reinterpret_cast<uint64_t>(ptr));
    }

// ============================================================================
// Hash Concepts
// ============================================================================

    // Concept for hashable types
    export template<typename T>
    concept Hashable = requires(T&& value) {
        { hash(std::forward<T>(value)) } -> std::convertible_to<uint64_t>;
    };
    
    // Concept for nothrow hashable types
    export template<typename T>
    concept NothrowHashable = Hashable<T> && 
        noexcept(hash(std::declval<T>()));

// ============================================================================
// Hash Utilities
// ============================================================================

    // Convenience function for hashing values
    export template<typename T>
    requires Hashable<T>
    constexpr uint64_t HashValue(T&& value) noexcept(NothrowHashable<T>) {
        return hash(std::forward<T>(value));
    }
    
    // Hash combination utility
    export template<typename... Ts>
    requires (Hashable<Ts> && ...)
    constexpr uint64_t HashCombine(const Ts&... values) noexcept {
        uint64_t seed = 0;
        ((seed = CombineHashes(seed, HashValue(values))), ...);
        return seed;
    }
    
    // Hash for ranges/containers
    export template<typename Range>
    requires requires(Range&& range) {
        std::ranges::begin(range);
        std::ranges::end(range);
    }
    constexpr uint64_t HashRange(Range&& range) noexcept {
        uint64_t hash = kFnvOffset;
        for (const auto& element : range) {
            hash = CombineHashes(hash, HashValue(element));
        }
        return hash;
    }

// ============================================================================
// Default Hasher for Containers
// ============================================================================

export template<typename T>
struct DefaultHasher {
    constexpr uint64_t operator()(const T& value) const noexcept {
        return HashValue(value);
    }
};

// Specialization for string_view
export template<>
struct DefaultHasher<std::string_view> {
    constexpr uint64_t operator()(std::string_view str) const noexcept {
        return Fnv1aString(str);
    }
};

// ============================================================================
// Type Signature Hash
// ============================================================================

export struct SignatureHasher {
    constexpr uint64_t Hash(std::string_view signature) const noexcept {
        return SimpleStringHash(signature);
    }
};
