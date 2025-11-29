export module Containers:Deque;
import Lang;
import Cap.Memory;
import <type_traits>;

export namespace Containers {
    template<typename T, typename AllocPolicy = Cap::Allocator>
    struct Deque {
        static constexpr USize kChunkBytes = 4096;
        T** m_chunks{ nullptr };
        USize m_chunkCount{ 0 };
        USize m_itemsPerChunk{ 0 };
        USize m_size{ 0 };
        USize m_headIndex{ 0 };
        USize m_mask{ 0 };
        USize m_chunkMask{ 0 };
        USize m_chunkShift{ 0 };
        Cap::SystemMemoryResource m_defaultRes{};
        AllocPolicy m_alloc{};

        constexpr Deque() noexcept { if constexpr (std::is_same_v<AllocPolicy, Cap::Allocator>) { m_alloc = AllocPolicy(&m_defaultRes); } }
        constexpr explicit Deque(AllocPolicy a) noexcept : m_alloc(a) {}
        Deque(const Deque&) = delete;
        Deque& operator=(const Deque&) = delete;
        ~Deque() noexcept { Clear(); ReleaseStorage(); }

        [[nodiscard]] USize Size() const noexcept { return m_size; }
        [[nodiscard]] USize Capacity() const noexcept { return m_chunkCount * m_itemsPerChunk; }

        [[nodiscard]] Status Reserve(USize cap) noexcept {
            auto NextPow2 = [](USize x) noexcept -> USize { if (x <= 1) return 1; --x; x |= x>>1; x |= x>>2; x |= x>>4; x |= x>>8; x |= x>>16; if constexpr (sizeof(USize)==8) { x |= x>>32; } return x+1; };
            USize perRaw = (kChunkBytes / static_cast<USize>(sizeof(T))); if (perRaw == 0) perRaw = 1;
            m_itemsPerChunk = NextPow2(perRaw);
            USize needChunks = (cap + m_itemsPerChunk - 1) / m_itemsPerChunk;
            USize needChunksPow2 = NextPow2(needChunks == 0 ? 1 : needChunks);
            if (needChunks <= m_chunkCount) return Ok(StatusDomain::System());
            auto pb = m_alloc.Alloc(needChunksPow2 * static_cast<USize>(sizeof(T*)), alignof(T*));
            if (!pb.IsOk()) return pb.Error();
            T** newChunks = static_cast<T**>(pb.Value().ptr);
            for (USize i = 0; i < needChunksPow2; ++i) {
                if (i < m_chunkCount && m_chunks && m_chunks[i]) { newChunks[i] = m_chunks[i]; continue; }
                auto cb = m_alloc.Alloc(m_itemsPerChunk * static_cast<USize>(sizeof(T)), alignof(T));
                if (!cb.IsOk()) { return cb.Error(); }
                newChunks[i] = static_cast<T*>(cb.Value().ptr);
            }
            if (m_chunks) {
                Cap::MemoryBlock blk{ m_chunks, m_chunkCount * static_cast<USize>(sizeof(T*)) };
                m_alloc.Free(blk, alignof(T*));
            }
            m_chunks = newChunks; m_chunkCount = needChunksPow2;
            m_mask = (m_chunkCount * m_itemsPerChunk) ? ((m_chunkCount * m_itemsPerChunk) - 1) : 0;
            m_chunkMask = m_itemsPerChunk ? (m_itemsPerChunk - 1) : 0;
            USize sh = 0; USize v = m_itemsPerChunk; while (v > 1) { v >>= 1; ++sh; } m_chunkShift = sh;
            return Ok(StatusDomain::System());
        }

        [[nodiscard]] T& operator[](USize i) noexcept { auto idx = MapIndex(i); return m_chunks[idx.c][idx.o]; }
        [[nodiscard]] const T& operator[](USize i) const noexcept { auto idx = MapIndex(i); return m_chunks[idx.c][idx.o]; }

        [[nodiscard]] Status PushBack(const T& v) noexcept { return EmplaceBack(v); }
        [[nodiscard]] Status PushBack(T&& v) noexcept { return EmplaceBack(Move(v)); }
        [[nodiscard]] Status PushFront(const T& v) noexcept { return EmplaceFront(v); }
        [[nodiscard]] Status PushFront(T&& v) noexcept { return EmplaceFront(Move(v)); }

        template<typename... Args>
        [[nodiscard]] Status EmplaceBack(Args&&... args) noexcept {
            if (m_size == Capacity()) { auto s = Reserve(Capacity() ? Capacity() * 2 : m_itemsPerChunk ? m_itemsPerChunk : (kChunkBytes / static_cast<USize>(sizeof(T)))); if (!s.Ok()) return s; }
            auto idx = MapIndex(m_size);
            ConstructAt<T>(static_cast<void*>(m_chunks[idx.c] + idx.o), Forward<Args>(args)...);
            ++m_size;
            return Ok(StatusDomain::System());
        }

        template<typename... Args>
        [[nodiscard]] Status EmplaceFront(Args&&... args) noexcept {
            if (m_size == Capacity()) { auto s = Reserve(Capacity() ? Capacity() * 2 : m_itemsPerChunk ? m_itemsPerChunk : (kChunkBytes / static_cast<USize>(sizeof(T)))); if (!s.Ok()) return s; }
            if (Capacity()) { m_headIndex = (m_headIndex - 1) & m_mask; }
            auto idx = MapIndex(0);
            ConstructAt<T>(static_cast<void*>(m_chunks[idx.c] + idx.o), Forward<Args>(args)...);
            ++m_size;
            return Ok(StatusDomain::System());
        }

        void PopBack() noexcept { if (m_size == 0) return; auto idx = MapIndex(m_size - 1); if constexpr (!TriviallyDestructible<T>) { m_chunks[idx.c][idx.o].~T(); } --m_size; }
        void PopFront() noexcept { if (m_size == 0) return; auto idx = MapIndex(0); if constexpr (!TriviallyDestructible<T>) { m_chunks[idx.c][idx.o].~T(); } if (Capacity()) { m_headIndex = (m_headIndex + 1) & m_mask; } --m_size; }

        void Clear() noexcept { while (m_size) PopBack(); }

        void ReleaseStorage() noexcept {
            if (!m_chunks) return;
            for (USize i = 0; i < m_chunkCount; ++i) {
                if (m_chunks[i]) {
                    Cap::MemoryBlock blk{ m_chunks[i], m_itemsPerChunk * static_cast<USize>(sizeof(T)) };
                    m_alloc.Free(blk, alignof(T));
                }
            }
            {
                Cap::MemoryBlock blk{ m_chunks, m_chunkCount * static_cast<USize>(sizeof(T*)) };
                m_alloc.Free(blk, alignof(T*));
            }
            m_chunks = nullptr; m_chunkCount = 0; m_itemsPerChunk = 0; m_size = 0; m_headIndex = 0;
        }

    private:
        struct Idx { USize c; USize o; };
        [[nodiscard]] Idx MapIndex(USize i) const noexcept {
            USize cap = Capacity();
            if (cap == 0) { return Idx{ 0, 0 }; }
            USize gi = (m_headIndex + i) & m_mask;
            USize c = (m_itemsPerChunk == 0) ? 0 : (gi >> m_chunkShift);
            USize o = (m_itemsPerChunk == 0) ? gi : (gi & m_chunkMask);
            return Idx{ c, o };
        }

        struct Iterator { Deque* d; USize i; [[nodiscard]] T& operator*() const noexcept { auto idx = d->MapIndex(i); return d->m_chunks[idx.c][idx.o]; } [[nodiscard]] T* operator->() const noexcept { auto idx = d->MapIndex(i); return d->m_chunks[idx.c] + idx.o; } Iterator& operator++() noexcept { ++i; return *this; } Iterator operator++(int) noexcept { Iterator tmp{*this}; ++i; return tmp; } [[nodiscard]] bool operator!=(const Iterator& rhs) const noexcept { return i != rhs.i; } [[nodiscard]] bool operator==(const Iterator& rhs) const noexcept { return i == rhs.i; } };
        struct ConstIterator { const Deque* d; USize i; [[nodiscard]] const T& operator*() const noexcept { auto idx = d->MapIndex(i); return d->m_chunks[idx.c][idx.o]; } [[nodiscard]] const T* operator->() const noexcept { auto idx = d->MapIndex(i); return d->m_chunks[idx.c] + idx.o; } ConstIterator& operator++() noexcept { ++i; return *this; } ConstIterator operator++(int) noexcept { ConstIterator tmp{*this}; ++i; return tmp; } [[nodiscard]] bool operator!=(const ConstIterator& rhs) const noexcept { return i != rhs.i; } [[nodiscard]] bool operator==(const ConstIterator& rhs) const noexcept { return i == rhs.i; } };
        [[nodiscard]] Iterator begin() noexcept { return Iterator{ this, 0 }; }
        [[nodiscard]] Iterator end() noexcept { return Iterator{ this, m_size }; }
        [[nodiscard]] ConstIterator begin() const noexcept { return ConstIterator{ this, 0 }; }
        [[nodiscard]] ConstIterator end() const noexcept { return ConstIterator{ this, m_size }; }
    };
}
