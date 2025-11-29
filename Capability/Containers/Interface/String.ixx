export module Containers:String;
import Lang;
import Cap.Memory;
import <type_traits>;

export namespace Containers {
    template<typename AllocPolicy = Cap::Allocator>
    struct String {
        static constexpr USize kSSOCap = 24;
        Cap::SystemMemoryResource m_defaultRes{};
        AllocPolicy m_alloc{};
        Char8* m_data{ nullptr };
        USize m_size{ 0 };
        USize m_capacity{ 0 };
        Char8 m_sso[kSSOCap]{};

        String() noexcept : m_data(m_sso), m_capacity(kSSOCap - 1) { if constexpr (std::is_same_v<AllocPolicy, Cap::Allocator>) { m_alloc = AllocPolicy(&m_defaultRes); } }
        explicit String(AllocPolicy a) noexcept : m_alloc(a), m_data(m_sso), m_capacity(kSSOCap - 1) {}
        String(const String&) = delete;
        String& operator=(const String&) = delete;
        ~String() noexcept { ReleaseHeap(); }

        [[nodiscard]] USize Size() const noexcept { return m_size; }
        [[nodiscard]] USize Capacity() const noexcept { return m_capacity; }
        [[nodiscard]] const Char8* Data() const noexcept { return m_data; }
        [[nodiscard]] Char8* Data() noexcept { return m_data; }
        [[nodiscard]] const Char8* CStr() noexcept { EnsureTerminated(); return m_data; }

        [[nodiscard]] Status Reserve(USize n) noexcept {
            USize need = n + 1;
            if (need <= m_capacity) return Ok(StatusDomain::System());
            USize newCap = need;
            // 简化：直接新分配并拷贝，避免后端差异导致的模块解析问题
            auto nb = m_alloc.Alloc(newCap, alignof(Char8));
            if (!nb.IsOk()) return nb.Error();
            void* np = nb.Value().ptr;
            if (m_size) { MemMove(np, m_data, m_size); }
            if (m_data != m_sso && m_capacity) {
                Cap::MemoryBlock blk{ m_data, m_capacity + 1 };
                m_alloc.Free(blk, alignof(Char8));
            }
            m_data = static_cast<Char8*>(np);
            m_capacity = newCap - 1;
            EnsureTerminated();
            return Ok(StatusDomain::System());
        }

        void Clear() noexcept { m_size = 0; EnsureTerminated(); }

        [[nodiscard]] Status Append(const Char8* s, USize n) noexcept {
            if (!s || n == 0) return Ok(StatusDomain::System());
            USize need = m_size + n + 1;
            if (need > m_capacity) {
                auto st = Reserve(need - 1);
                if (!st.Ok()) return st;
            }
            MemMove(m_data + m_size, s, n);
            m_size += n;
            EnsureTerminated();
            return Ok(StatusDomain::System());
        }

        [[nodiscard]] Status Append(StringView sv) noexcept { return Append(sv.data(), static_cast<USize>(sv.size())); }

        [[nodiscard]] Status Assign(const Char8* s, USize n) noexcept { m_size = 0; return Append(s, n); }
        [[nodiscard]] Status Assign(StringView sv) noexcept { m_size = 0; return Append(sv.data(), static_cast<USize>(sv.size())); }

    private:
        void EnsureTerminated() noexcept { if (m_data) { m_data[m_size] = static_cast<Char8>(0); } }
        void ReleaseHeap() noexcept {
            if (m_data != m_sso && m_data) {
                Cap::MemoryBlock blk{ m_data, m_capacity + 1 };
                m_alloc.Free(blk, alignof(Char8));
            }
            m_data = m_sso; m_capacity = kSSOCap - 1;
        }
    };
}
