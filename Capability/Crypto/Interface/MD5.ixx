export module Cap.Crypto:MD5;

export namespace Cap::Crypto {

    class MD5Hasher final : public Hasher {
    public:
        MD5Hasher() noexcept;
        ~MD5Hasher() noexcept override = default;
        
        void Append(const void* data, size_t size) noexcept override;
        HashResult Finalize() noexcept override;
        void Reset() noexcept override;
        size_t GetDigestSize() const noexcept override;
    
    private:
        void ProcessBlock(const uint8_t* block) noexcept;
        
        uint8_t m_Buffer[64];
        uint64_t m_BitLength;
        uint32_t m_State[4];
        uint8_t m_Digest[16];
        size_t m_BufferSize;
    };

    HashResult MD5(const void* data, size_t size, IMemoryResource* allocator = nullptr) noexcept;
}
