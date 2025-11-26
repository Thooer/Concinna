export module Tools.Tracy;

export namespace Tools::Tracy {
    struct Zone {
        void* p{};
        explicit Zone(const char* name) noexcept;
        ~Zone() noexcept;
        Zone(const Zone&) = delete;
        Zone& operator=(const Zone&) = delete;
        Zone(Zone&&) = delete;
        Zone& operator=(Zone&&) = delete;
    };
    void SetThreadName(const char* name) noexcept;
    void FrameMark() noexcept;
    void FrameMarkStart(const char* name) noexcept;
    void FrameMarkEnd(const char* name) noexcept;
    void Message(const char* text) noexcept;
    void Plot(const char* name, double value) noexcept;
    void Plot(const char* name, float value) noexcept;
    void Plot(const char* name, long long value) noexcept;
    void Plot(const char* name, unsigned long long value) noexcept;
    void Plot(const char* name, int value) noexcept;
    void Plot(const char* name, unsigned int value) noexcept;
}