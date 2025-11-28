module Cap.Crypto:MD5;

import Cap.Crypto:MD5;

namespace Cap::Crypto {

    // MD5 constants
    constexpr uint32_t MD5_S[64] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };

    constexpr uint32_t MD5_K[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    // Rotate left function
    inline uint32_t RotateLeft(uint32_t x, uint32_t n) noexcept {
        return (x << n) | (x >> (32 - n));
    }

    // MD5Hasher implementation
    MD5Hasher::MD5Hasher() noexcept {
        Reset();
    }

    void MD5Hasher::Reset() noexcept {
        // Initial hash values (little-endian)
        m_State[0] = 0x67452301;
        m_State[1] = 0xefcdab89;
        m_State[2] = 0x98badcfe;
        m_State[3] = 0x10325476;
        
        m_BitLength = 0;
        m_BufferSize = 0;
    }

    void MD5Hasher::ProcessBlock(const uint8_t* block) noexcept {
        uint32_t a = m_State[0];
        uint32_t b = m_State[1];
        uint32_t c = m_State[2];
        uint32_t d = m_State[3];
        
        uint32_t w[16];
        
        // Convert block to little-endian words
        for (size_t i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(block[i * 4]) & 0xFF) |
                   ((static_cast<uint32_t>(block[i * 4 + 1]) & 0xFF) << 8) |
                   ((static_cast<uint32_t>(block[i * 4 + 2]) & 0xFF) << 16) |
                   ((static_cast<uint32_t>(block[i * 4 + 3]) & 0xFF) << 24);
        }
        
        // Main loop
        for (size_t i = 0; i < 64; ++i) {
            uint32_t f, g;
            
            if (i < 16) {
                f = (b & c) | (~b & d);
                g = i;
            } else if (i < 32) {
                f = (d & b) | (~d & c);
                g = (5 * i + 1) % 16;
            } else if (i < 48) {
                f = b ^ c ^ d;
                g = (3 * i + 5) % 16;
            } else {
                f = c ^ (b | ~d);
                g = (7 * i) % 16;
            }
            
            uint32_t temp = d;
            d = c;
            c = b;
            b = b + RotateLeft((a + f + MD5_K[i] + w[g]), MD5_S[i]);
            a = temp;
        }
        
        // Update hash state
        m_State[0] += a;
        m_State[1] += b;
        m_State[2] += c;
        m_State[3] += d;
    }

    void MD5Hasher::Append(const void* data, size_t size) noexcept {
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

    HashResult MD5Hasher::Finalize() noexcept {
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
        
        // Append bit length as little-endian
        for (size_t i = 0; i < 8; ++i) {
            m_Buffer[56 + i] = static_cast<uint8_t>((m_BitLength >> (i * 8)) & 0xFF);
        }
        
        ProcessBlock(m_Buffer);
        
        // Convert hash state to little-endian digest
        for (size_t i = 0; i < 4; ++i) {
            m_Digest[i * 4] = static_cast<uint8_t>(m_State[i] & 0xFF);
            m_Digest[i * 4 + 1] = static_cast<uint8_t>((m_State[i] >> 8) & 0xFF);
            m_Digest[i * 4 + 2] = static_cast<uint8_t>((m_State[i] >> 16) & 0xFF);
            m_Digest[i * 4 + 3] = static_cast<uint8_t>((m_State[i] >> 24) & 0xFF);
        }
        
        return { m_Digest, sizeof(m_Digest) };
    }

    size_t MD5Hasher::GetDigestSize() const noexcept {
        return sizeof(m_Digest);
    }

    HashResult MD5(const void* data, size_t size, IMemoryResource* allocator) noexcept {
        MD5Hasher hasher;
        hasher.Append(data, size);
        return hasher.Finalize();
    }
}
