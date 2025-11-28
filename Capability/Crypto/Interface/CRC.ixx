export module Cap.Crypto:CRC;

export namespace Cap::Crypto {

    uint32_t CRC32(const void* data, size_t size) noexcept;
    uint32_t CRC32(const void* data, size_t size, uint32_t initial) noexcept;
    
    uint64_t CRC64(const void* data, size_t size) noexcept;
    uint64_t CRC64(const void* data, size_t size, uint64_t initial) noexcept;

    class CRC32Hasher final : public Hasher {
    public:
        CRC32Hasher() noexcept;
        ~CRC32Hasher() noexcept override = default;
        
        void Append(const void* data, size_t size) noexcept override;
        HashResult Finalize() noexcept override;
        void Reset() noexcept override;
        size_t GetDigestSize() const noexcept override;
    
    private:
        uint32_t m_Crc;
        uint8_t m_Digest[4];
    };

    class CRC64Hasher final : public Hasher {
    public:
        CRC64Hasher() noexcept;
        ~CRC64Hasher() noexcept override = default;
        
        void Append(const void* data, size_t size) noexcept override;
        HashResult Finalize() noexcept override;
        void Reset() noexcept override;
        size_t GetDigestSize() const noexcept override;
    
    private:
        uint64_t m_Crc;
        uint8_t m_Digest[8];
    };
}
