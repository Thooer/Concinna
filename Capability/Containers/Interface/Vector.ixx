export module Containers:Vector;

import Language;
import Cap.Memory;
import :Traits;
import <type_traits>;

export namespace Containers {
    template<typename AllocPolicy>
    struct RawVector {
        void* m_data{ nullptr };
        USize m_size{ 0 };
        USize m_capacity{ 0 };
        Cap::SystemMemoryResource m_defaultRes{};
        AllocPolicy m_alloc{};
        USize m_align{ 1 };
        USize m_elemSize{ 1 };

        RawVector() noexcept = default;
        RawVector(USize elemSize, USize align) noexcept : m_align(align), m_elemSize(elemSize) {
            if constexpr (std::is_same_v<AllocPolicy, Cap::Allocator>) { m_alloc = AllocPolicy(&m_defaultRes); }
        }
        RawVector(USize elemSize, USize align, AllocPolicy a) noexcept : m_alloc(a), m_align(align), m_elemSize(elemSize) {}

        [[nodiscard]] Status TryReallocate(USize newCap) noexcept {
            if (!m_data) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
            if constexpr (std::is_same_v<AllocPolicy, Cap::Allocator>) {
                const USize newBytes = newCap * m_elemSize;
                const USize oldBytes = m_capacity * m_elemSize;
                if (newBytes > oldBytes) {
                    if (m_alloc.TryExpand(m_data, oldBytes, newBytes - oldBytes, m_align)) {
                        m_capacity = newCap;
                        return Ok(StatusDomain::System());
                    }
                }
                auto rb = m_alloc.resource->Reallocate(Cap::MemoryBlock{ m_data, oldBytes }, newBytes, m_align);
                if (!rb.IsOk()) return rb.Error();
                m_data = rb.Value().ptr;
                m_capacity = newCap;
                return Ok(StatusDomain::System());
            } else {
                return Err(StatusDomain::System(), StatusCode::Failed);
            }
        }

        [[nodiscard]] auto Alloc(USize cap) noexcept { return m_alloc.Alloc(cap * m_elemSize, m_align); }
        void Free() noexcept {
            if (!m_data || m_capacity == 0) return;
            m_alloc.Free(Cap::MemoryBlock{ m_data, m_capacity * m_elemSize }, m_align);
            m_data = nullptr; m_capacity = 0; m_size = 0;
        }
    };
    template<typename T, typename AllocPolicy = Cap::Allocator>
    struct Vector {
        RawVector<AllocPolicy> m_raw{ sizeof(T), alignof(T) };

        Vector() noexcept : m_raw(sizeof(T), alignof(T)) {}
        explicit Vector(AllocPolicy a) noexcept : m_raw(sizeof(T), alignof(T), a) {}
        Vector(const Vector&) = delete;
        Vector& operator=(const Vector&) = delete;
        ~Vector() noexcept { Clear(); ReleaseStorage(); }

        [[nodiscard]] constexpr USize Size() const noexcept { return m_raw.m_size; }
        [[nodiscard]] constexpr USize Capacity() const noexcept { return m_raw.m_capacity; }
        [[nodiscard]] constexpr T* Data() noexcept { return static_cast<T*>(m_raw.m_data); }
        [[nodiscard]] constexpr const T* Data() const noexcept { return static_cast<const T*>(m_raw.m_data); }

        T& operator[](USize i) noexcept { return static_cast<T*>(m_raw.m_data)[i]; }
        const T& operator[](USize i) const noexcept { return static_cast<const T*>(m_raw.m_data)[i]; }

        [[nodiscard]] T& GetUnchecked(USize i) noexcept { return static_cast<T*>(m_raw.m_data)[i]; }
        [[nodiscard]] const T& GetUnchecked(USize i) const noexcept { return static_cast<const T*>(m_raw.m_data)[i]; }

        [[nodiscard]] Status Reserve(USize newCap) noexcept {
            if (newCap <= m_raw.m_capacity) return Ok(StatusDomain::System());
            const USize bytes = newCap * static_cast<USize>(sizeof(T));
            if (m_raw.m_data && Containers::TriviallyRelocatable<T>) {
                auto sr = m_raw.TryReallocate(newCap);
                if (sr.Ok()) { return sr; }
            }
            auto nb = m_raw.m_alloc.Alloc(bytes, m_raw.m_align);
            if (!nb.IsOk()) return nb.Error();
            void* newPtr = nb.Value().ptr;
            if (m_raw.m_data) {
                if constexpr (Containers::TriviallyRelocatable<T>) {
                    MemMove(newPtr, m_raw.m_data, m_raw.m_size * static_cast<USize>(sizeof(T)));
                } else {
                    auto* dst = static_cast<T*>(newPtr);
                    USize constructed = 0;
#if CFG_EXCEPTIONS
                    try {
                        for (USize i = 0; i < m_raw.m_size; ++i) { ConstructAt<T>(static_cast<void*>(dst + i), Move(static_cast<T*>(m_raw.m_data)[i])); ++constructed; }
                    } catch (...) {
                        for (USize i = 0; i < constructed; ++i) { DestroyAt<T>(dst + i); }
                        m_raw.m_alloc.Free(Cap::MemoryBlock{ newPtr, bytes }, m_raw.m_align);
                        return Err(StatusDomain::System(), StatusCode::Failed);
                    }
#else
                    for (USize i = 0; i < m_raw.m_size; ++i) { ConstructAt<T>(static_cast<void*>(dst + i), Move(static_cast<T*>(m_raw.m_data)[i])); }
#endif
                    for (USize i = 0; i < m_raw.m_size; ++i) { DestroyAt<T>(static_cast<T*>(m_raw.m_data) + i); }
                }
                m_raw.m_alloc.Free(Cap::MemoryBlock{ m_raw.m_data, m_raw.m_capacity * static_cast<USize>(sizeof(T)) }, m_raw.m_align);
            }
            m_raw.m_data = newPtr;
            m_raw.m_capacity = newCap;
            return Ok(StatusDomain::System());
        }

        template<typename... Args>
        [[nodiscard]] Status EmplaceBack(Args&&... args) noexcept {
            if (m_raw.m_size == m_raw.m_capacity) {
                USize grow = (m_raw.m_capacity == 0) ? 4 : (m_raw.m_capacity * 2);
                Status s = Reserve(grow);
                if (!s.Ok()) return s;
            }
            ConstructAt<T>(static_cast<void*>(static_cast<T*>(m_raw.m_data) + m_raw.m_size), Forward<Args>(args)...);
            ++m_raw.m_size;
            return Ok(StatusDomain::System());
        }

        template<typename... Args>
        void EmplaceBackUnsafe(Args&&... args) noexcept {
            if (m_raw.m_size == m_raw.m_capacity) {
                USize grow = (m_raw.m_capacity == 0) ? 4 : (m_raw.m_capacity * 2);
                Status s = Reserve(grow);
                if (!s.Ok()) { DebugBreak(); return; }
            }
            ConstructAt<T>(static_cast<void*>(static_cast<T*>(m_raw.m_data) + m_raw.m_size), Forward<Args>(args)...);
            ++m_raw.m_size;
        }

        [[nodiscard]] Status PushBack(const T& v) noexcept { return EmplaceBack(v); }
        [[nodiscard]] Status PushBack(T&& v) noexcept { return EmplaceBack(Move(v)); }

        void Clear() noexcept {
            if (!m_raw.m_data) { m_raw.m_size = 0; return; }
            for (USize i = 0; i < m_raw.m_size; ++i) { DestroyAt<T>(static_cast<T*>(m_raw.m_data) + i); }
            m_raw.m_size = 0;
        }

        void PopBack() noexcept {
            if (m_raw.m_size == 0) return;
            --m_raw.m_size;
            DestroyAt<T>(static_cast<T*>(m_raw.m_data) + m_raw.m_size);
        }

        [[nodiscard]] Status Resize(USize newSize) noexcept {
            if (newSize > m_raw.m_capacity) {
                USize grow = newSize;
                auto s = Reserve(grow);
                if (!s.Ok()) return s;
            }
            if (newSize > m_raw.m_size) {
                for (USize i = m_raw.m_size; i < newSize; ++i) { ConstructAt<T>(static_cast<void*>(static_cast<T*>(m_raw.m_data) + i)); }
            } else {
                for (USize i = newSize; i < m_raw.m_size; ++i) { DestroyAt<T>(static_cast<T*>(m_raw.m_data) + i); }
            }
            m_raw.m_size = newSize;
            return Ok(StatusDomain::System());
        }

        void SetLengthUnsafe(USize len) noexcept { m_raw.m_size = len; }

        void Erase(USize index) noexcept {
            if (index >= m_raw.m_size) return;
            if constexpr (!TriviallyDestructible<T>) { static_cast<T*>(m_raw.m_data)[index].~T(); }
            if (index + 1 < m_raw.m_size) {
                if constexpr (Containers::TriviallyRelocatable<T>) {
                    MemMove(static_cast<T*>(m_raw.m_data) + index, static_cast<T*>(m_raw.m_data) + index + 1, (m_raw.m_size - index - 1) * static_cast<USize>(sizeof(T)));
                } else {
                    for (USize i = index; i + 1 < m_raw.m_size; ++i) { static_cast<T*>(m_raw.m_data)[i] = Move(static_cast<T*>(m_raw.m_data)[i + 1]); }
                }
            }
            --m_raw.m_size;
        }

        void EraseUnordered(USize index) noexcept {
            if (index >= m_raw.m_size) return;
            USize last = m_raw.m_size - 1;
            if (index != last) {
                if constexpr (Containers::TriviallyRelocatable<T>) {
                    MemMove(static_cast<T*>(m_raw.m_data) + index, static_cast<T*>(m_raw.m_data) + last, static_cast<USize>(sizeof(T)));
                } else {
                    static_cast<T*>(m_raw.m_data)[index] = Move(static_cast<T*>(m_raw.m_data)[last]);
                }
            }
            if constexpr (!TriviallyDestructible<T>) { static_cast<T*>(m_raw.m_data)[last].~T(); }
            --m_raw.m_size;
        }

        void ReleaseStorage() noexcept { m_raw.Free(); }

        void GrowByUninitialized(USize delta) noexcept { m_raw.m_size += delta; }

        [[nodiscard]] Status InsertHole(USize index, USize count) noexcept {
            if (count == 0) return Ok(StatusDomain::System());
            if (index > m_raw.m_size) return Err(StatusDomain::System(), StatusCode::OutOfRange);
            USize oldSize = m_raw.m_size;
            USize newSize = oldSize + count;
            if (newSize > m_raw.m_capacity) {
                auto s = Reserve(newSize);
                if (!s.Ok()) return s;
            }
            if (index < oldSize) {
                if constexpr (Containers::TriviallyRelocatable<T>) {
                    MemMove(static_cast<T*>(m_raw.m_data) + index + count,
                                      static_cast<T*>(m_raw.m_data) + index,
                                      (oldSize - index) * static_cast<USize>(sizeof(T)));
                } else {
                    for (USize i = oldSize; i > index; --i) {
                        static_cast<T*>(m_raw.m_data)[i + count - 1] = Move(static_cast<T*>(m_raw.m_data)[i - 1]);
                    }
                }
            }
            m_raw.m_size = newSize;
            return Ok(StatusDomain::System());
        }

        [[nodiscard]] Status AppendRange(const T* src, USize count) noexcept {
            if (!src || count == 0) return Ok(StatusDomain::System());
            USize oldSize = m_raw.m_size;
            USize need = oldSize + count;
            USize aliasOffset = 0;
            bool aliased = false;
            if (m_raw.m_data && src >= static_cast<const T*>(m_raw.m_data) && src < static_cast<const T*>(m_raw.m_data) + oldSize) {
                aliased = true;
                aliasOffset = static_cast<USize>(src - static_cast<const T*>(m_raw.m_data));
            }
            if (need > m_raw.m_capacity) {
                auto s = Reserve(need);
                if (!s.Ok()) return s;
            }
            const T* copySrc = src;
            if (aliased) { copySrc = static_cast<const T*>(m_raw.m_data) + aliasOffset; }
            if constexpr (Containers::TriviallyRelocatable<T>) {
                MemMove(static_cast<T*>(m_raw.m_data) + oldSize,
                                  copySrc,
                                  count * static_cast<USize>(sizeof(T)));
                m_raw.m_size = need;
                return Ok(StatusDomain::System());
            } else {
                for (USize i = 0; i < count; ++i) {
                    ConstructAt<T>(static_cast<void*>(static_cast<T*>(m_raw.m_data) + oldSize + i), copySrc[i]);
                }
                m_raw.m_size = need;
                return Ok(StatusDomain::System());
            }
        }

        [[nodiscard]] Status AppendRange(const Vector& other) noexcept {
            return AppendRange(other.Data(), other.Size());
        }

        [[nodiscard]] bool Equals(const Vector& rhs) const noexcept {
            if (m_raw.m_size != rhs.m_raw.m_size) return false;
            if (m_raw.m_size == 0) return true;
            if constexpr (Containers::TriviallyComparable<T>) {
                const auto n = m_raw.m_size * static_cast<USize>(sizeof(T));
                const auto* a = static_cast<const Byte*>(static_cast<const void*>(static_cast<const T*>(m_raw.m_data)));
                const auto* b = static_cast<const Byte*>(static_cast<const void*>(rhs.Data()));
                for (USize i = 0; i < n; ++i) { if (a[i] != b[i]) return false; }
                return true;
            } else if constexpr (requires (const T& x, const T& y) { x == y; }) {
                for (USize i = 0; i < m_raw.m_size; ++i) { if (!(static_cast<const T*>(m_raw.m_data)[i] == rhs.Data()[i])) return false; }
                return true;
            } else {
                return m_raw.m_data == rhs.m_raw.m_data;
            }
        }

        struct Iterator { T* ptr; Iterator& advance() noexcept { return *this; } [[nodiscard]] T& operator*() const noexcept { return *ptr; } Iterator& operator++() noexcept { ++ptr; return *this; } };
        struct ConstIterator { const T* ptr; ConstIterator& advance() noexcept { return *this; } [[nodiscard]] const T& operator*() const noexcept { return *ptr; } ConstIterator& operator++() noexcept { ++ptr; return *this; } };
        struct Sentinel { const T* end_ptr; };
        [[nodiscard]] friend bool operator!=(const Iterator& it, const Sentinel& s) noexcept { return it.ptr != s.end_ptr; }
        [[nodiscard]] friend bool operator!=(const ConstIterator& it, const Sentinel& s) noexcept { return it.ptr != s.end_ptr; }
        [[nodiscard]] auto begin() noexcept { return Iterator{ static_cast<T*>(m_raw.m_data) }; }
        [[nodiscard]] auto end() noexcept { return Sentinel{ static_cast<const T*>(m_raw.m_data) + m_raw.m_size }; }
        [[nodiscard]] auto begin() const noexcept { return ConstIterator{ static_cast<const T*>(m_raw.m_data) }; }
        [[nodiscard]] auto end() const noexcept { return Sentinel{ static_cast<const T*>(m_raw.m_data) + m_raw.m_size }; }
    };

    template<typename T, USize N, typename AllocPolicy = Memory::Allocator>
    struct SmallVector {
        RawVector<AllocPolicy> m_raw{ sizeof(T), alignof(T) };
        alignas(T) Byte m_sbo[N * static_cast<USize>(sizeof(T))]{};
        bool m_sboActive{ true };

        SmallVector() noexcept : m_raw(sizeof(T), alignof(T)) { m_raw.m_data = m_sbo; m_raw.m_capacity = N; }
        explicit SmallVector(AllocPolicy a) noexcept : m_raw(sizeof(T), alignof(T), a) { m_raw.m_data = m_sbo; m_raw.m_capacity = N; }
        SmallVector(const SmallVector&) = delete;
        SmallVector& operator=(const SmallVector&) = delete;
        ~SmallVector() noexcept { Clear(); ReleaseStorage(); }

        [[nodiscard]] constexpr USize Size() const noexcept { return m_raw.m_size; }
        [[nodiscard]] constexpr USize Capacity() const noexcept { return m_raw.m_capacity; }
        [[nodiscard]] constexpr T* Data() noexcept { return static_cast<T*>(m_raw.m_data); }
        [[nodiscard]] constexpr const T* Data() const noexcept { return static_cast<const T*>(m_raw.m_data); }

        T& operator[](USize i) noexcept { return static_cast<T*>(m_raw.m_data)[i]; }
        const T& operator[](USize i) const noexcept { return static_cast<const T*>(m_raw.m_data)[i]; }

        [[nodiscard]] Status Reserve(USize newCap) noexcept {
            if (newCap <= m_raw.m_capacity) return Ok(StatusDomain::System());
            const USize bytes = newCap * static_cast<USize>(sizeof(T));
            auto nb = m_raw.m_alloc.Alloc(bytes, m_raw.m_align);
            if (!nb.IsOk()) return nb.Error();
            void* newPtr = nb.Value().ptr;
            if (m_sboActive) {
                if constexpr (Containers::TriviallyRelocatable<T>) {
                    MemMove(newPtr, m_raw.m_data, m_raw.m_size * static_cast<USize>(sizeof(T)));
                } else {
                    auto* dst = static_cast<T*>(newPtr);
                    for (USize i = 0; i < m_raw.m_size; ++i) { ConstructAt<T>(static_cast<void*>(dst + i), Move(static_cast<T*>(m_raw.m_data)[i])); }
                    for (USize i = 0; i < m_raw.m_size; ++i) { DestroyAt<T>(static_cast<T*>(m_raw.m_data) + i); }
                }
                m_raw.m_data = newPtr;
                m_raw.m_capacity = newCap;
                m_sboActive = false;
                return Ok(StatusDomain::System());
            }
            if (m_raw.m_data) {
                if constexpr (Containers::TriviallyRelocatable<T>) {
                    MemMove(newPtr, m_raw.m_data, m_raw.m_size * static_cast<USize>(sizeof(T)));
                } else {
                    auto* dst = static_cast<T*>(newPtr);
                    for (USize i = 0; i < m_raw.m_size; ++i) { ConstructAt<T>(static_cast<void*>(dst + i), Move(static_cast<T*>(m_raw.m_data)[i])); }
                    for (USize i = 0; i < m_raw.m_size; ++i) { DestroyAt<T>(static_cast<T*>(m_raw.m_data) + i); }
                }
                m_raw.m_alloc.Free(Cap::MemoryBlock{ m_raw.m_data, m_raw.m_capacity * static_cast<USize>(sizeof(T)) }, m_raw.m_align);
            }
            m_raw.m_data = newPtr;
            m_raw.m_capacity = newCap;
            return Ok(StatusDomain::System());
        }

        template<typename... Args>
        [[nodiscard]] Status EmplaceBack(Args&&... args) noexcept {
            if (m_raw.m_size == m_raw.m_capacity) {
                USize grow = (m_raw.m_capacity == 0) ? 4 : (m_raw.m_capacity * 2);
                Status s = Reserve(grow);
                if (!s.Ok()) return s;
            }
            ConstructAt<T>(static_cast<void*>(static_cast<T*>(m_raw.m_data) + m_raw.m_size), Forward<Args>(args)...);
            ++m_raw.m_size;
            return Ok(StatusDomain::System());
        }

        template<typename... Args>
        void EmplaceBackUnsafe(Args&&... args) noexcept {
            if (m_raw.m_size == m_raw.m_capacity) {
                USize grow = (m_raw.m_capacity == 0) ? 4 : (m_raw.m_capacity * 2);
                Status s = Reserve(grow);
                if (!s.Ok()) { DebugBreak(); return; }
            }
            ConstructAt<T>(static_cast<void*>(static_cast<T*>(m_raw.m_data) + m_raw.m_size), Forward<Args>(args)...);
            ++m_raw.m_size;
        }

        [[nodiscard]] Status PushBack(const T& v) noexcept { return EmplaceBack(v); }
        [[nodiscard]] Status PushBack(T&& v) noexcept { return EmplaceBack(Move(v)); }

        void Clear() noexcept {
            if (!m_raw.m_data) { m_raw.m_size = 0; return; }
            for (USize i = 0; i < m_raw.m_size; ++i) { DestroyAt<T>(static_cast<T*>(m_raw.m_data) + i); }
            m_raw.m_size = 0;
        }

        void PopBack() noexcept {
            if (m_raw.m_size == 0) return;
            --m_raw.m_size;
            DestroyAt<T>(static_cast<T*>(m_raw.m_data) + m_raw.m_size);
        }

        [[nodiscard]] Status Resize(USize newSize) noexcept {
            if (newSize > m_raw.m_capacity) {
                USize grow = newSize;
                auto s = Reserve(grow);
                if (!s.Ok()) return s;
            }
            if (newSize > m_raw.m_size) {
                for (USize i = m_raw.m_size; i < newSize; ++i) { ConstructAt<T>(static_cast<void*>(static_cast<T*>(m_raw.m_data) + i)); }
            } else {
                for (USize i = newSize; i < m_raw.m_size; ++i) { DestroyAt<T>(static_cast<T*>(m_raw.m_data) + i); }
            }
            m_raw.m_size = newSize;
            return Ok(StatusDomain::System());
        }

        void Erase(USize index) noexcept {
            if (index >= m_raw.m_size) return;
            if constexpr (!TriviallyDestructible<T>) { static_cast<T*>(m_raw.m_data)[index].~T(); }
            if (index + 1 < m_raw.m_size) {
                if constexpr (Containers::TriviallyRelocatable<T>) {
                    MemMove(static_cast<T*>(m_raw.m_data) + index, static_cast<T*>(m_raw.m_data) + index + 1, (m_raw.m_size - index - 1) * static_cast<USize>(sizeof(T)));
                } else {
                    for (USize i = index; i + 1 < m_raw.m_size; ++i) { static_cast<T*>(m_raw.m_data)[i] = Move(static_cast<T*>(m_raw.m_data)[i + 1]); }
                }
            }
            --m_raw.m_size;
        }

        void EraseUnordered(USize index) noexcept {
            if (index >= m_raw.m_size) return;
            USize last = m_raw.m_size - 1;
            if (index != last) {
                if constexpr (Containers::TriviallyRelocatable<T>) {
                    MemMove(static_cast<T*>(m_raw.m_data) + index, static_cast<T*>(m_raw.m_data) + last, static_cast<USize>(sizeof(T)));
                } else {
                    static_cast<T*>(m_raw.m_data)[index] = Move(static_cast<T*>(m_raw.m_data)[last]);
                }
            }
            if constexpr (!TriviallyDestructible<T>) { static_cast<T*>(m_raw.m_data)[last].~T(); }
            --m_raw.m_size;
        }

        void ReleaseStorage() noexcept { if (!m_sboActive) m_raw.Free(); }

        void GrowByUninitialized(USize delta) noexcept { m_raw.m_size += delta; }

        [[nodiscard]] Status InsertHole(USize index, USize count) noexcept {
            if (count == 0) return Ok(StatusDomain::System());
            if (index > m_raw.m_size) return Err(StatusDomain::System(), StatusCode::OutOfRange);
            USize oldSize = m_raw.m_size;
            USize newSize = oldSize + count;
            if (newSize > m_raw.m_capacity) {
                auto s = Reserve(newSize);
                if (!s.Ok()) return s;
            }
            if (index < oldSize) {
                if constexpr (Containers::TriviallyRelocatable<T>) {
                    MemMove(static_cast<T*>(m_raw.m_data) + index + count,
                                      static_cast<T*>(m_raw.m_data) + index,
                                      (oldSize - index) * static_cast<USize>(sizeof(T)));
                } else {
                    for (USize i = oldSize; i > index; --i) {
                        static_cast<T*>(m_raw.m_data)[i + count - 1] = Move(static_cast<T*>(m_raw.m_data)[i - 1]);
                    }
                }
            }
            m_raw.m_size = newSize;
            return Ok(StatusDomain::System());
        }

        [[nodiscard]] Status AppendRange(const T* src, USize count) noexcept {
            if (!src || count == 0) return Ok(StatusDomain::System());
            USize oldSize = m_raw.m_size;
            USize need = oldSize + count;
            if (need > m_raw.m_capacity) {
                auto s = Reserve(need);
                if (!s.Ok()) return s;
            }
            if constexpr (Containers::TriviallyRelocatable<T>) {
                MemMove(static_cast<T*>(m_raw.m_data) + oldSize, src, count * static_cast<USize>(sizeof(T)));
                m_raw.m_size = need;
                return Ok(StatusDomain::System());
            } else {
                for (USize i = 0; i < count; ++i) { ConstructAt<T>(static_cast<void*>(static_cast<T*>(m_raw.m_data) + oldSize + i), src[i]); }
                m_raw.m_size = need;
                return Ok(StatusDomain::System());
            }
        }

        [[nodiscard]] Status AppendRange(const SmallVector& other) noexcept { return AppendRange(other.Data(), other.Size()); }

        struct Iterator { T* ptr; Iterator& advance() noexcept { return *this; } [[nodiscard]] T& operator*() const noexcept { return *ptr; } Iterator& operator++() noexcept { ++ptr; return *this; } };
        struct ConstIterator { const T* ptr; ConstIterator& advance() noexcept { return *this; } [[nodiscard]] const T& operator*() const noexcept { return *ptr; } ConstIterator& operator++() noexcept { ++ptr; return *this; } };
        struct Sentinel { const T* end_ptr; };
        [[nodiscard]] friend bool operator!=(const Iterator& it, const Sentinel& s) noexcept { return it.ptr != s.end_ptr; }
        [[nodiscard]] friend bool operator!=(const ConstIterator& it, const Sentinel& s) noexcept { return it.ptr != s.end_ptr; }
        [[nodiscard]] auto begin() noexcept { return Iterator{ static_cast<T*>(m_raw.m_data) }; }
        [[nodiscard]] auto end() noexcept { return Sentinel{ static_cast<const T*>(m_raw.m_data) + m_raw.m_size }; }
        [[nodiscard]] auto begin() const noexcept { return ConstIterator{ static_cast<const T*>(m_raw.m_data) }; }
        [[nodiscard]] auto end() const noexcept { return Sentinel{ static_cast<const T*>(m_raw.m_data) + m_raw.m_size }; }
    };
}
