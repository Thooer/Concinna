module;

export module Test:Registration;

import Language;

import :Core;

export namespace Test {

    struct TestEntry {
        const char* category{nullptr};
        const char* name{nullptr};
        TestFunction fn{nullptr};
        bool frameTest{false};
    };

    class ITestRegistry {
    public:
        virtual ~ITestRegistry() = default;
        virtual void Register(const TestEntry& entry) noexcept = 0;
        [[nodiscard]] virtual USize Count() const noexcept = 0;
        [[nodiscard]] virtual const TestEntry& Get(USize index) const noexcept = 0;
    };

    class StaticRegistry final : public ITestRegistry {
    public:
        void Register(const TestEntry& entry) noexcept override {
            if (entry.fn == nullptr || m_count >= kMaxEntries) { return; }
            m_entries[m_count++] = entry;
        }

        [[nodiscard]] USize Count() const noexcept override { return m_count; }
        [[nodiscard]] const TestEntry& Get(USize index) const noexcept override { return m_entries[index]; }

        static StaticRegistry& Instance() noexcept {
            static StaticRegistry s_registry{};
            return s_registry;
        }

    private:
        static constexpr USize kMaxEntries = 4096;
        TestEntry m_entries[kMaxEntries]{};
        USize m_count{0};
    };

    inline ITestRegistry& Registry() noexcept { return StaticRegistry::Instance(); }

    inline void Register(const char* category,
                         const char* name,
                         TestFunction fn,
                         bool frameTest = false) noexcept {
        StaticRegistry::Instance().Register(TestEntry{category, name, fn, frameTest});
    }

    struct AutoRegister {
        AutoRegister(const char* category,
                     const char* name,
                     TestFunction fn,
                     bool frameTest = false) noexcept {
            Register(category, name, fn, frameTest);
        }
    };

    using FailFastHandler = void(*)() noexcept;
    inline thread_local FailFastHandler g_failFast{nullptr};

    inline void SetFailFastHandler(FailFastHandler fn) noexcept { g_failFast = fn; }

    inline void FailFast() noexcept {
        if (g_failFast) { g_failFast(); }
    }
}
