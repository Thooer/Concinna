export module Containers:Intrusive;

import Language;

export namespace Containers {
    template<typename T>
    struct IntrusiveNode { T* next{nullptr}; };

    template<typename T>
    struct IntrusiveList {
        T* head{nullptr};
        T* tail{nullptr};

        [[nodiscard]] bool Empty() const noexcept { return head == nullptr; }

        void PushBack(T* node) noexcept {
            if (!node) return;
            node->next = nullptr;
            if (!tail) { head = tail = node; } else { tail->next = node; tail = node; }
        }

        [[nodiscard]] T* PopFront() noexcept {
            T* n = head;
            if (!n) return nullptr;
            head = n->next;
            if (!head) tail = nullptr;
            n->next = nullptr;
            return n;
        }

        void Clear() noexcept { while (head) { T* n = head; head = n->next; n->next = nullptr; } tail = nullptr; }
    };
}