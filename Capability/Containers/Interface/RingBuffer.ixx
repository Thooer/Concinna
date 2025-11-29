export module Containers:RingBuffer;

import Lang;
import Cap.Memory;

export namespace Containers {
    template<typename T>
    struct RawRingBuffer {
        T* m_data{nullptr};
        USize m_capacity{0};
        USize m_mask{0};
        USize m_head{0};
        USize m_tail{0};

        void Init(T* buffer, USize capacity) noexcept {
            if (!buffer || capacity == 0) { m_data = nullptr; m_capacity = 0; m_mask = 0; m_head = m_tail = 0; return; }
            // 要求容量为 2 的幂，以便使用按位与掩码
            if (!Cap::Alignment::IsPowerOfTwo(capacity)) { m_data = nullptr; m_capacity = 0; m_mask = 0; m_head = m_tail = 0; return; }
            m_data = buffer; m_capacity = capacity; m_mask = capacity - 1; m_head = m_tail = 0;
        }

        [[nodiscard]] bool Valid() const noexcept { return m_data != nullptr && m_capacity != 0; }
        [[nodiscard]] bool Empty() const noexcept { return m_head == m_tail; }
        [[nodiscard]] bool Full() const noexcept { return ((m_tail + 1) & m_mask) == m_head; }
        [[nodiscard]] USize Size() const noexcept { return (m_tail - m_head) & m_mask; }
        [[nodiscard]] USize Capacity() const noexcept { return m_capacity; }

        [[nodiscard]] bool PushBack(const T& v) noexcept {
            if (!Valid()) return false;
            if (Full()) return false;
            m_data[m_tail] = v;
            m_tail = (m_tail + 1) & m_mask;
            return true;
        }
        [[nodiscard]] bool PushBack(T&& v) noexcept {
            if (!Valid()) return false;
            if (Full()) return false;
            m_data[m_tail] = Move(v);
            m_tail = (m_tail + 1) & m_mask;
            return true;
        }

        [[nodiscard]] bool PopFront(T& out) noexcept {
            if (!Valid()) return false;
            if (Empty()) return false;
            out = Move(m_data[m_head]);
            m_head = (m_head + 1) & m_mask;
            return true;
        }
    };
}
