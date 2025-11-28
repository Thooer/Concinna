export module Cap.Crypto:Hash;

export import Cap.Memory;

export namespace Cap::Crypto {

    enum class HashType {
        CRC32,
        CRC64,
        MD5,
        SHA256,
        BLAKE3
    };

    struct HashResult {
        const uint8_t* Data;
        size_t Size;
    };

    class Hasher {
    public:
        virtual ~Hasher() = default;
        
        virtual void Append(const void* data, size_t size) noexcept = 0;
        virtual HashResult Finalize() noexcept = 0;
        virtual void Reset() noexcept = 0;
        virtual size_t GetDigestSize() const noexcept = 0;
    };

    Hasher* CreateHasher(HashType type, IMemoryResource* allocator = nullptr) noexcept;
    void DestroyHasher(Hasher* hasher) noexcept;

    template<typename T>
    HashResult ComputeHash(HashType type, const T& data, IMemoryResource* allocator = nullptr) noexcept {
        Hasher* hasher = CreateHasher(type, allocator);
        if (!hasher) {
            return { nullptr, 0 };
        }
        
        hasher->Append(&data, sizeof(T));
        HashResult result = hasher->Finalize();
        DestroyHasher(hasher);
        
        return result;
    }

    template<typename T>
    HashResult ComputeHash(HashType type, const T* data, size_t count, IMemoryResource* allocator = nullptr) noexcept {
        Hasher* hasher = CreateHasher(type, allocator);
        if (!hasher) {
            return { nullptr, 0 };
        }
        
        hasher->Append(data, count * sizeof(T));
        HashResult result = hasher->Finalize();
        DestroyHasher(hasher);
        
        return result;
    }

    HashResult ComputeHash(HashType type, const void* data, size_t size, IMemoryResource* allocator = nullptr) noexcept;
}
