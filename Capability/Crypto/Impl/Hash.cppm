module Cap.Crypto:Hash;

import Cap.Crypto:Hash;
import Cap.Crypto:CRC;
import Cap.Crypto:SHA256;
import Cap.Crypto:MD5;
import Cap.Memory;

namespace Cap::Crypto {

    Hasher* CreateHasher(HashType type, IMemoryResource* allocator) noexcept {
        if (!allocator) {
            allocator = GetSystemAllocator();
        }

        switch (type) {
            case HashType::CRC32:
                return New<CRC32Hasher>(*allocator);
            case HashType::CRC64:
                return New<CRC64Hasher>(*allocator);
            case HashType::MD5:
                return New<MD5Hasher>(*allocator);
            case HashType::SHA256:
                return New<SHA256Hasher>(*allocator);
            case HashType::BLAKE3:
                // TODO: Implement BLAKE3
                return nullptr;
            default:
                return nullptr;
        }
    }

    void DestroyHasher(Hasher* hasher) noexcept {
        if (hasher) {
            Delete(hasher);
        }
    }

    HashResult ComputeHash(HashType type, const void* data, size_t size, IMemoryResource* allocator) noexcept {
        Hasher* hasher = CreateHasher(type, allocator);
        if (!hasher) {
            return { nullptr, 0 };
        }
        
        hasher->Append(data, size);
        HashResult result = hasher->Finalize();
        DestroyHasher(hasher);
        
        return result;
    }
}
