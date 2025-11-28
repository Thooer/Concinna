export module Cap.Crypto:SHA256;

export namespace Cap::Crypto {

    class SHA256Hasher final : public Hasher {
    public:
        SHA256Hasher() noexcept;
        ~SHA256Hasher() noexcept override = default;
        
        void Append(const void* data, size_t size) noexcept override;
        HashResult Finalize() noexcept override;
        void Reset() noexcept override;
        size_t GetDigestSize() const noexcept override;
    
    private:
        void ProcessBlock(const uint8_t* block) noexcept;
        
        uint8_t m_Buffer[64];
        uint64_t m_BitLength;
        uint32_t m_State[8];
        uint8_t m_Digest[32];
        size_t m_BufferSize;
    };

    HashResult SHA256(const void* data, size_t size, IMemoryResource* allocator = nullptr) noexcept;
}
