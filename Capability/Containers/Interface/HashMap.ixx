export module Containers:HashMap;

import Language;
import SIMD;
import Memory;
import :String;
import :Traits;

export namespace Containers {
    template<typename K, typename V, typename Hasher, typename AllocPolicy>
    struct HashMapIterator;
    template<typename K, typename V, typename Hasher, typename AllocPolicy>
    struct HashMapConstIterator;
    
    // Use Hash module's DefaultHasher
    template<typename K>
    using DefaultHasher = DefaultHasher<K>;

    template<typename K, typename V, typename Hasher = DefaultHasher<K>, typename AllocPolicy = Memory::Allocator>
    struct HashMap {
        static constexpr USize kGroupSize = 8;
        K* m_keys{ nullptr };
        V* m_vals{ nullptr };
        UInt8* m_ctrl{ nullptr };
        USize m_size{ 0 };
        USize m_capacity{ 0 };
        USize m_mask{ 0 };
        Memory::SystemMemoryResource m_defaultRes{};
        AllocPolicy m_alloc{};
        Memory::MemoryBlock m_storage{};
        Hasher m_hasher{};

        constexpr HashMap() noexcept { if constexpr (SameAs<AllocPolicy, Memory::Allocator>) { m_alloc = AllocPolicy(&m_defaultRes); } }
        constexpr explicit HashMap(AllocPolicy a) noexcept : m_alloc(a) {}
        HashMap(const HashMap&) = delete;
        HashMap& operator=(const HashMap&) = delete;
        ~HashMap() noexcept { Clear(); ReleaseStorage(); }

        [[nodiscard]] USize Size() const noexcept { return m_size; }
        [[nodiscard]] USize Capacity() const noexcept { return m_capacity; }

        [[nodiscard]] Status Reserve(USize cap) noexcept {
            USize c = 1;
            while (c < cap) c <<= 1;
            if (c <= m_capacity) return Ok(StatusDomain::System());
            USize ctrlBytes = c * static_cast<USize>(sizeof(UInt8));
            USize keysBytes = c * static_cast<USize>(sizeof(K));
            USize valsBytes = c * static_cast<USize>(sizeof(V));
            USize a = 1;
            a = a < static_cast<USize>(alignof(K)) ? static_cast<USize>(alignof(K)) : a;
            a = a < static_cast<USize>(alignof(V)) ? static_cast<USize>(alignof(V)) : a;
            auto nb = m_alloc.Alloc(ctrlBytes + keysBytes + valsBytes + a * 2, a);
            if (!nb.IsOk()) return nb.Error();
            auto base = static_cast<Byte*>(nb.Value().ptr);
            UIntPtr p = reinterpret_cast<UIntPtr>(base + static_cast<SSize>(ctrlBytes));
            UIntPtr kptr = Memory::Alignment::AlignUp(p, static_cast<USize>(alignof(K)));
            UIntPtr vptr = Memory::Alignment::AlignUp(kptr + keysBytes, static_cast<USize>(alignof(V)));
            UInt8* ns = base;
            K* nk = reinterpret_cast<K*>(kptr);
            V* nv = reinterpret_cast<V*>(vptr);
            for (USize i = 0; i < c; ++i) ns[i] = 0;
            if (m_size) {
                for (USize i = 0; i < m_capacity; ++i) {
                    if (m_ctrl[i] > 0 && m_ctrl[i] != 0xFF) { InsertInto(nk, nv, ns, c, m_keys[i], m_vals[i]); }
                }
                if (m_storage.ptr) {
                    m_alloc.Free(m_storage, a);
                } else {
                    m_alloc.Free(Memory::MemoryBlock{ m_ctrl, m_capacity * static_cast<USize>(sizeof(UInt8)) }, alignof(UInt8));
                    m_alloc.Free(Memory::MemoryBlock{ m_keys, m_capacity * static_cast<USize>(sizeof(K)) }, alignof(K));
                    m_alloc.Free(Memory::MemoryBlock{ m_vals, m_capacity * static_cast<USize>(sizeof(V)) }, alignof(V));
                }
            }
            m_storage = Memory::MemoryBlock{ base, static_cast<USize>(nb.Value().size) };
            m_ctrl = ns; m_keys = nk; m_vals = nv; m_capacity = c; m_mask = c - 1;
            return Ok(StatusDomain::System());
        }

        [[nodiscard]] Status Put(const K& key, const V& val) noexcept {
            if ((m_size + 1) * 10 >= m_capacity * 7) {
                auto s = Reserve(m_capacity ? m_capacity * 2 : 8);
                if (!s.Ok()) return s;
            }
            bool inserted = InsertInto(m_keys, m_vals, m_ctrl, m_capacity, key, val);
            if (inserted) ++m_size;
            return Ok(StatusDomain::System());
        }

        [[nodiscard]] V* GetPtr(const K& key) noexcept {
            if (m_capacity == 0) return nullptr;
            UInt64 h = m_hasher(key);
            USize pos = static_cast<USize>(h) & m_mask;
            USize start = pos & ~(kGroupSize - 1);
            UInt8 h2 = static_cast<UInt8>((h & 0x7Fu) | 1u);
            for (USize step = 0; step < m_capacity; step += kGroupSize) {
                USize group = (start + step) & m_mask;
                UInt8 eq = SIMD::EqMask8(m_ctrl + group, h2);
                while (eq) {
                    USize bit = FirstSetBit(eq);
                    USize i = group + bit;
                    if (EqualKey(m_keys[i], key)) return m_vals + i;
                    eq &= static_cast<UInt8>(eq - (1u << bit));
                }
                UInt8 empty = SIMD::EqMask8(m_ctrl + group, 0u);
                if (empty) return nullptr;
            }
            return nullptr;
        }

        [[nodiscard]] V* GetPtr(StringView key) noexcept {
            if (m_capacity == 0) return nullptr;
            UInt64 h = m_hasher(key);
            USize pos = static_cast<USize>(h) & m_mask;
            USize start = pos & ~(kGroupSize - 1);
            UInt8 h2 = static_cast<UInt8>((h & 0x7Fu) | 1u);
            for (USize step = 0; step < m_capacity; step += kGroupSize) {
                USize group = (start + step) & m_mask;
                UInt8 eq = SIMD::EqMask8(m_ctrl + group, h2);
                while (eq) {
                    USize bit = FirstSetBit(eq);
                    USize i = group + bit;
                    if (EqualKey(m_keys[i], key)) return m_vals + i;
                    eq &= static_cast<UInt8>(eq - (1u << bit));
                }
                UInt8 empty = SIMD::EqMask8(m_ctrl + group, 0u);
                if (empty) return nullptr;
            }
            return nullptr;
        }

        [[nodiscard]] V* GetPtr(const char* cstr) noexcept { return GetPtr(StringView{ cstr }); }

        [[nodiscard]] bool Erase(const K& key) noexcept {
            if (m_capacity == 0) return false;
            UInt64 h = m_hasher(key);
            USize pos = static_cast<USize>(h) & m_mask;
            USize start = pos & ~(kGroupSize - 1);
            UInt8 h2 = static_cast<UInt8>((h & 0x7Fu) | 1u);
            for (USize step = 0; step < m_capacity; step += kGroupSize) {
                USize group = (start + step) & m_mask;
                UInt8 eq = SIMD::EqMask8(m_ctrl + group, h2);
                while (eq) {
                    USize bit = FirstSetBit(eq);
                    USize i = group + bit;
                    if (EqualKey(m_keys[i], key)) { m_ctrl[i] = 0xFF; --m_size; return true; }
                    eq &= static_cast<UInt8>(eq - (1u << bit));
                }
                UInt8 empty = SIMD::EqMask8(m_ctrl + group, 0u);
                if (empty) return false;
            }
            return false;
        }

        [[nodiscard]] bool Erase(StringView key) noexcept {
            if (m_capacity == 0) return false;
            UInt64 h = m_hasher(key);
            USize pos = static_cast<USize>(h) & m_mask;
            USize start = pos & ~(kGroupSize - 1);
            UInt8 h2 = static_cast<UInt8>((h & 0x7Fu) | 1u);
            for (USize step = 0; step < m_capacity; step += kGroupSize) {
                USize group = (start + step) & m_mask;
                UInt8 eq = SIMD::EqMask8(m_ctrl + group, h2);
                while (eq) {
                    USize bit = FirstSetBit(eq);
                    USize i = group + bit;
                    if (EqualKey(m_keys[i], key)) { m_ctrl[i] = 0xFF; --m_size; return true; }
                    eq &= static_cast<UInt8>(eq - (1u << bit));
                }
                UInt8 empty = SIMD::EqMask8(m_ctrl + group, 0u);
                if (empty) return false;
            }
            return false;
        }

        template<typename... Args>
        [[nodiscard]] bool TryEmplace(const K& key, Args&&... args) noexcept {
            if ((m_size + 1) * 10 >= m_capacity * 7) {
                auto s = Reserve(m_capacity ? m_capacity * 2 : 8);
                if (!s.Ok()) return false;
            }
            bool inserted = InsertIntoEmplace(m_keys, m_vals, m_ctrl, m_capacity, key, Forward<Args>(args)...);
            if (inserted) ++m_size;
            return inserted;
        }

        template<typename F>
        [[nodiscard]] V* GetOrInsert(const K& key, F&& factory) noexcept {
            if (m_capacity == 0) {
                auto s = Reserve(8);
                if (!s.Ok()) return nullptr;
            }
            UInt64 h = m_hasher(key);
            UInt8 h2 = static_cast<UInt8>((h & 0x7Fu) | 1u);
            USize pos = static_cast<USize>(h) & m_mask;
            USize start = pos & ~(kGroupSize - 1);
            USize firstTomb = m_capacity;
            for (USize step = 0; step < m_capacity; step += kGroupSize) {
                USize group = (start + step) & m_mask;
                UInt8 eq = SIMD::EqMask8(m_ctrl + group, h2);
                while (eq) {
                    USize bit = FirstSetBit(eq);
                    USize i = group + bit;
                    if (EqualKey(m_keys[i], key)) { return m_vals + i; }
                    eq &= static_cast<UInt8>(eq - (1u << bit));
                }
                UInt8 tomb = SIMD::EqMask8(m_ctrl + group, 0xFFu);
                if (tomb && firstTomb == m_capacity) { firstTomb = group + FirstSetBit(tomb); }
                UInt8 empty = SIMD::EqMask8(m_ctrl + group, 0u);
                if (empty) {
                    USize target = (firstTomb != m_capacity) ? firstTomb : (group + FirstSetBit(empty));
                    ConstructAt<K>(static_cast<void*>(m_keys + target), key);
                    V v = factory();
                    ConstructAt<V>(static_cast<void*>(m_vals + target), Move(v));
                    m_ctrl[target] = h2;
                    ++m_size;
                    return m_vals + target;
                }
            }
            auto rs = Reserve(m_capacity ? m_capacity * 2 : 8);
            if (!rs.Ok()) return nullptr;
            return GetOrInsert(key, Forward<F>(factory));
        }

        void Clear() noexcept {
            if (!m_ctrl) { m_size = 0; return; }
            for (USize i = 0; i < m_capacity; ++i) {
                if (m_ctrl[i] > 0 && m_ctrl[i] != 0xFF) {
                    if constexpr (!TriviallyDestructible<K>) { m_keys[i].~K(); }
                    if constexpr (!TriviallyDestructible<V>) { m_vals[i].~V(); }
                }
                m_ctrl[i] = 0;
            }
            m_size = 0;
        }

        void ReleaseStorage() noexcept {
            if (!m_ctrl) return;
            if (m_storage.ptr) {
                USize a = 1;
                a = a < static_cast<USize>(alignof(K)) ? static_cast<USize>(alignof(K)) : a;
                a = a < static_cast<USize>(alignof(V)) ? static_cast<USize>(alignof(V)) : a;
                m_alloc.Free(m_storage, a);
            } else {
                m_alloc.Free(Memory::MemoryBlock{ m_ctrl, m_capacity * static_cast<USize>(sizeof(UInt8)) }, alignof(UInt8));
                m_alloc.Free(Memory::MemoryBlock{ m_keys, m_capacity * static_cast<USize>(sizeof(K)) }, alignof(K));
                m_alloc.Free(Memory::MemoryBlock{ m_vals, m_capacity * static_cast<USize>(sizeof(V)) }, alignof(V));
            }
            m_storage = Memory::MemoryBlock{};
            m_ctrl = nullptr; m_keys = nullptr; m_vals = nullptr; m_capacity = 0; m_mask = 0;
        }

        [[nodiscard]] auto begin() noexcept { return HashMapIterator<K,V,Hasher,AllocPolicy>{ this, 0 }.advance(); }
        [[nodiscard]] auto end() noexcept { return HashMapIterator<K,V,Hasher,AllocPolicy>{ this, m_capacity }; }
        [[nodiscard]] auto begin() const noexcept { return HashMapConstIterator<K,V,Hasher,AllocPolicy>{ this, 0 }.advance(); }
        [[nodiscard]] auto end() const noexcept { return HashMapConstIterator<K,V,Hasher,AllocPolicy>{ this, m_capacity }; }
    private:
        [[nodiscard]] static bool EqualBytes(const void* pa, const void* pb, USize n) noexcept {
            const auto* a = static_cast<const Byte*>(pa);
            const auto* b = static_cast<const Byte*>(pb);
            for (USize i = 0; i < n; ++i) { if (a[i] != b[i]) return false; }
            return true;
        }
        [[nodiscard]] bool EqualKey(const K& a, const K& b) const noexcept {
            if constexpr (Containers::TriviallyComparable<K>) {
                return EqualBytes(&a, &b, static_cast<USize>(sizeof(K)));
            } else if constexpr (requires { a == b; }) {
                return a == b;
            } else {
                return &a == &b;
            }
        }
        [[nodiscard]] bool EqualKey(const K& a, StringView sv) const noexcept {
            if constexpr (::Containers::AllocatorAware<K>::value) {
                StringView ak{ a.Data(), a.Size() };
                return ak.Equals(sv);
            } else if constexpr (requires { a == sv; }) {
                return a == sv;
            } else {
                return false;
            }
        }
        [[nodiscard]] bool InsertInto(K* keys, V* vals, UInt8* ctrl, USize cap, const K& key, const V& val) noexcept {
            UInt64 h = m_hasher(key);
            UInt8 h2 = static_cast<UInt8>((h & 0x7Fu) | 1u);
            USize pos = static_cast<USize>(h) & (cap - 1);
            USize start = pos & ~(kGroupSize - 1);
            USize firstTomb = cap;
            for (USize step = 0; step < cap; step += kGroupSize) {
                USize group = (start + step) & (cap - 1);
                UInt8 eq = SIMD::EqMask8(ctrl + group, h2);
                while (eq) {
                    USize bit = FirstSetBit(eq);
                    USize i = group + bit;
                    if (EqualKey(keys[i], key)) { vals[i] = val; return false; }
                    eq &= static_cast<UInt8>(eq - (1u << bit));
                }
                UInt8 tomb = SIMD::EqMask8(ctrl + group, 0xFFu);
                if (tomb && firstTomb == cap) { firstTomb = group + FirstSetBit(tomb); }
                UInt8 empty = SIMD::EqMask8(ctrl + group, 0u);
                if (empty) {
                    USize target = (firstTomb != cap) ? firstTomb : (group + FirstSetBit(empty));
                    ConstructAt<K>(static_cast<void*>(keys + target), key);
                    ConstructAt<V>(static_cast<void*>(vals + target), val);
                    ctrl[target] = h2;
                    return true;
                }
            }
            return false;
        }
        template<typename... Args>
        [[nodiscard]] bool InsertIntoEmplace(K* keys, V* vals, UInt8* ctrl, USize cap, const K& key, Args&&... args) noexcept {
            UInt64 h = m_hasher(key);
            UInt8 h2 = static_cast<UInt8>((h & 0x7Fu) | 1u);
            USize pos = static_cast<USize>(h) & (cap - 1);
            USize start = pos & ~(kGroupSize - 1);
            USize firstTomb = cap;
            for (USize step = 0; step < cap; step += kGroupSize) {
                USize group = (start + step) & (cap - 1);
                UInt8 eq = SIMD::EqMask8(ctrl + group, h2);
                while (eq) {
                    USize bit = FirstSetBit(eq);
                    USize i = group + bit;
                    if (EqualKey(keys[i], key)) { return false; }
                    eq &= static_cast<UInt8>(eq - (1u << bit));
                }
                UInt8 tomb = SIMD::EqMask8(ctrl + group, 0xFFu);
                if (tomb && firstTomb == cap) { firstTomb = group + FirstSetBit(tomb); }
                UInt8 empty = SIMD::EqMask8(ctrl + group, 0u);
                if (empty) {
                    USize target = (firstTomb != cap) ? firstTomb : (group + FirstSetBit(empty));
                    ConstructAt<K>(static_cast<void*>(keys + target), key);
                    ConstructAt<V>(static_cast<void*>(vals + target), Forward<Args>(args)...);
                    ctrl[target] = h2;
                    return true;
                }
            }
            return false;
        }

        static constexpr USize FirstSetBit(UInt8 m) noexcept {
            for (USize i = 0; i < 8; ++i) { if (m & static_cast<UInt8>(1u << i)) return i; }
            return 8;
        }

    };

    template<typename K, typename V, typename Hasher, typename AllocPolicy>
    struct HashMapIterator {
        HashMap<K,V,Hasher,AllocPolicy>* m;
        USize i;
        [[nodiscard]] bool Valid(USize idx) const noexcept { return m->m_ctrl && m->m_ctrl[idx] > 0 && m->m_ctrl[idx] != 0xFF; }
        HashMapIterator& advance() noexcept { while (i < m->m_capacity && !Valid(i)) ++i; return *this; }
        [[nodiscard]] auto operator*() const noexcept { struct KV { K& key; V& value; }; return KV{ m->m_keys[i], m->m_vals[i] }; }
        HashMapIterator& operator++() noexcept { ++i; return advance(); }
        [[nodiscard]] bool operator!=(const HashMapIterator& rhs) const noexcept { return i != rhs.i; }
    };
    template<typename K, typename V, typename Hasher, typename AllocPolicy>
    struct HashMapConstIterator {
        const HashMap<K,V,Hasher,AllocPolicy>* m;
        USize i;
        [[nodiscard]] bool Valid(USize idx) const noexcept { return m->m_ctrl && m->m_ctrl[idx] > 0 && m->m_ctrl[idx] != 0xFF; }
        HashMapConstIterator& advance() noexcept { while (i < m->m_capacity && !Valid(i)) ++i; return *this; }
        [[nodiscard]] auto operator*() const noexcept { struct KV { const K& key; const V& value; }; return KV{ m->m_keys[i], m->m_vals[i] }; }
        HashMapConstIterator& operator++() noexcept { ++i; return advance(); }
        [[nodiscard]] bool operator!=(const HashMapConstIterator& rhs) const noexcept { return i != rhs.i; }
    };
    template<typename K, typename V, typename Hasher, typename AllocPolicy>
    [[nodiscard]] auto begin(HashMap<K,V,Hasher,AllocPolicy>& m) noexcept { return HashMapIterator<K,V,Hasher,AllocPolicy>{ &m, 0 }.advance(); }
    template<typename K, typename V, typename Hasher, typename AllocPolicy>
    [[nodiscard]] auto end(HashMap<K,V,Hasher,AllocPolicy>& m) noexcept { return HashMapIterator<K,V,Hasher,AllocPolicy>{ &m, m.Capacity() }; }
    template<typename K, typename V, typename Hasher, typename AllocPolicy>
    [[nodiscard]] auto begin(const HashMap<K,V,Hasher,AllocPolicy>& m) noexcept { return HashMapConstIterator<K,V,Hasher,AllocPolicy>{ &m, 0 }.advance(); }
    template<typename K, typename V, typename Hasher, typename AllocPolicy>
    [[nodiscard]] auto end(const HashMap<K,V,Hasher,AllocPolicy>& m) noexcept { return HashMapConstIterator<K,V,Hasher,AllocPolicy>{ &m, m.Capacity() }; }
}