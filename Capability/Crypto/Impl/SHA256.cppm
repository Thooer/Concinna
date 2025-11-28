module Cap.Crypto:SHA256;

import Cap.Crypto:SHA256;

namespace Cap::Crypto {

    // SHA-256 constants
    constexpr uint32_t SHA256_K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    // Rotate right function
    inline uint32_t RotateRight(uint32_t x, uint32_t n) noexcept {
        return (x >> n) | (x << (32 - n));
    }

    // SHA-256 helper functions
    inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) noexcept {
        return (x & y) ^ (~x & z);
    }

    inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) noexcept {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    inline uint32_t Sigma0(uint32_t x) noexcept {
        return RotateRight(x, 2) ^ RotateRight(x, 13) ^ RotateRight(x, 22);
    }

    inline uint32_t Sigma1(uint32_t x) noexcept {
        return RotateRight(x, 6) ^ RotateRight(x, 11) ^ RotateRight(x, 25);
    }

    inline uint32_t sigma0(uint32_t x) noexcept {
        return RotateRight(x, 7) ^ RotateRight(x, 18) ^ (x >> 3);
    }

    inline uint32_t sigma1(uint32_t x) noexcept {
        return RotateRight(x, 17) ^ RotateRight(x, 19) ^ (x >> 10);
    }

    // SHA256Hasher implementation
    SHA256Hasher::SHA256Hasher() noexcept {
        Reset();
    }

    void SHA256Hasher::Reset() noexcept {
        // Initial hash values (first 32 bits of the fractional parts of the square roots of the first 8 primes)
        m_State[0] = 0x6a09e667;
        m_State[1] = 0xbb67ae85;
        m_State[2] = 0x3c6ef372;
        m_State[3] = 0xa54ff53a;
        m_State[4] = 0x510e527f;
        m_State[5] = 0x9b05688c;
        m_State[6] = 0x1f83d9ab;
        m_State[7] = 0x5be0cd19;
        
        m_BitLength = 0;
        m_BufferSize = 0;
    }

    void SHA256Hasher::ProcessBlock(const uint8_t* block) noexcept {
        uint32_t w[64];
        
        // Convert block to big-endian words
        for (size_t i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
                   (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
                   static_cast<uint32_t>(block[i * 4 + 3]);
        }
        
        // Extend the first 16 words to 64 words
        for (size_t i = 16; i < 64; ++i) {
            w[i] = sigma1(w[i - 2]) + w[i - 7] + sigma0(w[i - 15]) + w[i - 16];
        }
        
        // Initialize working variables
        uint32_t a = m_State[0];
        uint32_t b = m_State[1];
        uint32_t c = m_State[2];
        uint32_t d = m_State[3];
        uint32_t e = m_State[4];
        uint32_t f = m_State[5];
        uint32_t g = m_State[6];
        uint32_t h = m_State[7];
        
        // Main loop
        for (size_t i = 0; i < 64; ++i) {
            uint32_t t1 = h + Sigma1(e) + Ch(e, f, g) + SHA256_K[i] + w[i];
            uint32_t t2 = Sigma0(a) + Maj(a, b, c);
            
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        
        // Update hash state
        m_State[0] += a;
        m_State[1] += b;
        m_State[2] += c;
        m_State[3] += d;
        m_State[4] += e;
        m_State[5] += f;
        m_State[6] += g;
        m_State[7] += h;
    }

    void SHA256Hasher::Append(const void* data, size_t size) noexcept {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        
        // Process any remaining data in the buffer
        if (m_BufferSize > 0) {
            size_t to_copy = 64 - m_BufferSize;
            if (to_copy > size) {
                to_copy = size;
            }
            
            memcpy(m_Buffer + m_BufferSize, bytes, to_copy);
            m_BufferSize += to_copy;
            bytes += to_copy;
            size -= to_copy;
            
            if (m_BufferSize == 64) {
                ProcessBlock(m_Buffer);
                m_BitLength += 512;
                m_BufferSize = 0;
            }
        }
        
        // Process full blocks
        while (size >= 64) {
            ProcessBlock(bytes);
            m_BitLength += 512;
            bytes += 64;
            size -= 64;
        }
        
        // Store remaining data in buffer
        if (size > 0) {
            memcpy(m_Buffer + m_BufferSize, bytes, size);
            m_BufferSize += size;
        }
    }

    HashResult SHA256Hasher::Finalize() noexcept {
        // Update bit length with remaining bytes
        m_BitLength += m_BufferSize * 8;
        
        // Append the '1' bit
        m_Buffer[m_BufferSize++] = 0x80;
        
        // If there's not enough room for the bit length, process the block
        if (m_BufferSize > 56) {
            memset(m_Buffer + m_BufferSize, 0, 64 - m_BufferSize);
            ProcessBlock(m_Buffer);
            m_BufferSize = 0;
        }
        
        // Pad with zeros and append bit length
        memset(m_Buffer + m_BufferSize, 0, 56 - m_BufferSize);
        
        // Append bit length as big-endian
        for (size_t i = 0; i < 8; ++i) {
            m_Buffer[56 + i] = static_cast<uint8_t>((m_BitLength >> (56 - i * 8)) & 0xFF);
        }
        
        ProcessBlock(m_Buffer);
        
        // Convert hash state to big-endian digest
        for (size_t i = 0; i < 8; ++i) {
            m_Digest[i * 4] = static_cast<uint8_t>((m_State[i] >> 24) & 0xFF);
            m_Digest[i * 4 + 1] = static_cast<uint8_t>((m_State[i] >> 16) & 0xFF);
            m_Digest[i * 4 + 2] = static_cast<uint8_t>((m_State[i] >> 8) & 0xFF);
            m_Digest[i * 4 + 3] = static_cast<uint8_t>(m_State[i] & 0xFF);
        }
        
        return { m_Digest, sizeof(m_Digest) };
    }

    size_t SHA256Hasher::GetDigestSize() const noexcept {
        return sizeof(m_Digest);
    }

    HashResult SHA256(const void* data, size_t size, IMemoryResource* allocator) noexcept {
        SHA256Hasher hasher;
        hasher.Append(data, size);
        return hasher.Finalize();
    }
}
