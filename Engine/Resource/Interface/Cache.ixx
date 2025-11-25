module;
export module Engine.Resource:Cache;

import Language;
import Foundation.Memory;
import :Handle;

export namespace Engine::Resource {
    class LRUCache {
    public:
        explicit LRUCache(::Foundation::Memory::IAllocator* alloc, Language::USize capacity) noexcept
            : m_alloc(alloc), m_capacity(capacity), m_count(0), m_tick(0),
              m_keyPtrs(nullptr), m_keySizes(nullptr), m_handles(nullptr), m_ticks(nullptr) {}

        [[nodiscard]] Language::Optional<Handle> Find(Language::StringView key) const noexcept {
            Language::USize idx = findIndex(key);
            Language::Optional<Handle> r;
            if (idx != kNpos) r.Emplace(m_handles[static_cast<size_t>(idx)]);
            return r;
        }

        [[nodiscard]] bool Add(Language::StringView key, Handle h) noexcept {
            Language::USize idx = findIndex(key);
            if (idx != kNpos) {
                m_handles[static_cast<size_t>(idx)] = h;
                m_ticks[static_cast<size_t>(idx)] = ++m_tick;
                return true;
            }
            if (m_count == m_capacity) {
                if (Evict(1) == 0) return false;
            }
            if (m_count == 0) allocateArrays(8);
            else if (m_count == m_allocCap) growArrays();
            Language::USize len = key.size();
            auto ar = m_alloc->Allocate((len+1) * static_cast<Language::USize>(sizeof(Language::Char8)), static_cast<Language::USize>(alignof(Language::Char8)));
            if (!ar.IsOk()) return false;
            auto* p = reinterpret_cast<Language::Char8*>(ar.OkValue());
            const Language::Char8* src = key.data();
            for (Language::USize i=0;i<len;++i) p[static_cast<size_t>(i)] = src[static_cast<size_t>(i)];
            p[static_cast<size_t>(len)] = static_cast<Language::Char8>(0);
            m_keyPtrs[static_cast<size_t>(m_count)] = p;
            m_keySizes[static_cast<size_t>(m_count)] = len;
            m_handles[static_cast<size_t>(m_count)] = h;
            m_ticks[static_cast<size_t>(m_count)] = ++m_tick;
            m_count++;
            return true;
        }

        void Touch(Language::StringView key) noexcept {
            Language::USize idx = findIndex(key);
            if (idx != kNpos) { m_ticks[static_cast<size_t>(idx)] = ++m_tick; }
        }

        [[nodiscard]] Language::USize Evict(Language::USize maxCount) noexcept {
            if (m_count == 0 || maxCount == 0) return 0;
            Language::USize ev = 0;
            while (ev < maxCount && m_count > 0) {
                Language::USize lruIdx = 0; Language::UInt64 lruTick = m_ticks[0];
                for (Language::USize i=1;i<m_count;++i) {
                    Language::UInt64 t = m_ticks[static_cast<size_t>(i)];
                    if (t < lruTick) { lruTick = t; lruIdx = i; }
                }
                auto* kp = m_keyPtrs[static_cast<size_t>(lruIdx)];
                auto ks = m_keySizes[static_cast<size_t>(lruIdx)];
                if (kp) {
                    auto* freePtr = const_cast<Language::Byte*>(reinterpret_cast<const Language::Byte*>(kp));
                    (void)m_alloc->Deallocate(freePtr, (ks+1) * static_cast<Language::USize>(sizeof(Language::Char8)), static_cast<Language::USize>(alignof(Language::Char8)));
                }
                Language::USize last = m_count - 1;
                if (lruIdx != last) {
                    m_keyPtrs[static_cast<size_t>(lruIdx)] = m_keyPtrs[static_cast<size_t>(last)];
                    m_keySizes[static_cast<size_t>(lruIdx)] = m_keySizes[static_cast<size_t>(last)];
                    m_handles[static_cast<size_t>(lruIdx)] = m_handles[static_cast<size_t>(last)];
                    m_ticks[static_cast<size_t>(lruIdx)] = m_ticks[static_cast<size_t>(last)];
                }
                m_count--;
                ev++;
            }
            return ev;
        }

        [[nodiscard]] Language::USize Size() const noexcept { return m_count; }
        [[nodiscard]] Language::USize Capacity() const noexcept { return m_capacity; }

    private:
        static constexpr Language::USize kNpos = static_cast<Language::USize>(-1);
        ::Foundation::Memory::IAllocator* m_alloc{};
        Language::USize m_capacity{0};
        Language::USize m_count{0};
        Language::USize m_allocCap{0};
        Language::UInt64 m_tick{0};
        const Language::Char8** m_keyPtrs{nullptr};
        Language::USize* m_keySizes{nullptr};
        Handle* m_handles{nullptr};
        Language::UInt64* m_ticks{nullptr};

        void allocateArrays(Language::USize initCap) noexcept {
            m_allocCap = initCap;
            auto r1 = m_alloc->Allocate(m_allocCap * static_cast<Language::USize>(sizeof(const Language::Char8*)), static_cast<Language::USize>(alignof(const Language::Char8*)));
            if (!r1.IsOk()) return; m_keyPtrs = reinterpret_cast<const Language::Char8**>(r1.OkValue());
            auto r2 = m_alloc->Allocate(m_allocCap * static_cast<Language::USize>(sizeof(Language::USize)), static_cast<Language::USize>(alignof(Language::USize)));
            if (!r2.IsOk()) return; m_keySizes = reinterpret_cast<Language::USize*>(r2.OkValue());
            auto r3 = m_alloc->Allocate(m_allocCap * static_cast<Language::USize>(sizeof(Handle)), static_cast<Language::USize>(alignof(Handle)));
            if (!r3.IsOk()) return; m_handles = reinterpret_cast<Handle*>(r3.OkValue());
            auto r4 = m_alloc->Allocate(m_allocCap * static_cast<Language::USize>(sizeof(Language::UInt64)), static_cast<Language::USize>(alignof(Language::UInt64)));
            if (!r4.IsOk()) return; m_ticks = reinterpret_cast<Language::UInt64*>(r4.OkValue());
        }

        void growArrays() noexcept {
            Language::USize newCap = (m_allocCap==0)?8:(m_allocCap*2);
            {
                auto r = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(const Language::Char8*)), static_cast<Language::USize>(alignof(const Language::Char8*)));
                if (!r.IsOk()) return; auto* np = reinterpret_cast<const Language::Char8**>(r.OkValue());
                for (Language::USize i=0;i<m_count;++i) np[static_cast<size_t>(i)] = m_keyPtrs ? m_keyPtrs[static_cast<size_t>(i)] : nullptr;
                if (m_keyPtrs) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(const_cast<Language::Char8**>(m_keyPtrs)), m_allocCap * static_cast<Language::USize>(sizeof(const Language::Char8*)), static_cast<Language::USize>(alignof(const Language::Char8*)));
                m_keyPtrs = np;
            }
            {
                auto r = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(Language::USize)), static_cast<Language::USize>(alignof(Language::USize)));
                if (!r.IsOk()) return; auto* np = reinterpret_cast<Language::USize*>(r.OkValue());
                for (Language::USize i=0;i<m_count;++i) np[static_cast<size_t>(i)] = m_keySizes ? m_keySizes[static_cast<size_t>(i)] : 0;
                if (m_keySizes) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_keySizes), m_allocCap * static_cast<Language::USize>(sizeof(Language::USize)), static_cast<Language::USize>(alignof(Language::USize)));
                m_keySizes = np;
            }
            {
                auto r = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(Handle)), static_cast<Language::USize>(alignof(Handle)));
                if (!r.IsOk()) return; auto* np = reinterpret_cast<Handle*>(r.OkValue());
                for (Language::USize i=0;i<m_count;++i) np[static_cast<size_t>(i)] = m_handles ? m_handles[static_cast<size_t>(i)] : Handle{0};
                if (m_handles) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_handles), m_allocCap * static_cast<Language::USize>(sizeof(Handle)), static_cast<Language::USize>(alignof(Handle)));
                m_handles = np;
            }
            {
                auto r = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(Language::UInt64)), static_cast<Language::USize>(alignof(Language::UInt64)));
                if (!r.IsOk()) return; auto* np = reinterpret_cast<Language::UInt64*>(r.OkValue());
                for (Language::USize i=0;i<m_count;++i) np[static_cast<size_t>(i)] = m_ticks ? m_ticks[static_cast<size_t>(i)] : 0ull;
                if (m_ticks) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_ticks), m_allocCap * static_cast<Language::USize>(sizeof(Language::UInt64)), static_cast<Language::USize>(alignof(Language::UInt64)));
                m_ticks = np;
            }
            m_allocCap = newCap;
        }

        [[nodiscard]] Language::USize findIndex(Language::StringView key) const noexcept {
            for (Language::USize i=0;i<m_count;++i) {
                if (m_keySizes[static_cast<size_t>(i)] == key.size()) {
                    const Language::Char8* p = m_keyPtrs[static_cast<size_t>(i)];
                    const Language::Char8* s = key.data();
                    Language::USize n = key.size();
                    Language::USize j=0; for (;j<n;++j) { if (p[static_cast<size_t>(j)]!=s[static_cast<size_t>(j)]) break; }
                    if (j==n) return i;
                }
            }
            return kNpos;
        }
    };
}