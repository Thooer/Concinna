export module Cap.Algorithms:Sort;

import Language;
import :Policy;

export namespace Cap {
    template<typename It>
    void Sort(It first, It last) noexcept {
        if (first == last) return;
        auto n = static_cast<USize>(last - first);
        auto insertion_sort = [&](USize l, USize r) noexcept {
            for (USize i = l + 1; i <= r; ++i) {
                auto key = first[i];
                USize j = i;
                while (j > l && key < first[j - 1]) { first[j] = first[j - 1]; --j; }
                first[j] = key;
            }
        };
        auto median3 = [&](USize a, USize b, USize c) noexcept -> USize {
            auto va = first[a], vb = first[b], vc = first[c];
            if (vb < va) { auto t = va; va = vb; vb = t; USize ti = a; a = b; b = ti; }
            if (vc < vb) { auto t = vb; vb = vc; vc = t; USize ti = b; b = c; c = ti; }
            if (vb < va) { auto t = va; va = vb; vb = t; USize ti = a; a = b; b = ti; }
            return b;
        };
        auto partition = [&](USize l, USize r) noexcept -> USize {
            USize m = l + ((r - l) >> 1);
            USize pivotIdx = median3(l, m, r);
            auto pivot = first[pivotIdx];
            auto tmp = first[pivotIdx]; first[pivotIdx] = first[r]; first[r] = tmp;
            USize i = l;
            for (USize j = l; j < r; ++j) { if (first[j] < pivot) { auto t = first[i]; first[i] = first[j]; first[j] = t; ++i; } }
            auto t = first[i]; first[i] = first[r]; first[r] = t; return i;
        };
        auto heapify = [&](USize l, USize count, USize i) noexcept {
            for (;;) {
                USize largest = i;
                USize left = (i << 1) + 1;
                USize right = left + 1;
                if (left < count && first[l + largest] < first[l + left]) largest = left;
                if (right < count && first[l + largest] < first[l + right]) largest = right;
                if (largest == i) break;
                auto t = first[l + i]; first[l + i] = first[l + largest]; first[l + largest] = t;
                i = largest;
            }
        };
        auto heap_sort = [&](USize l, USize r) noexcept {
            USize count = r - l + 1;
            for (USize i = (count >> 1); i > 0; --i) { heapify(l, count, i - 1); }
            for (USize end = count - 1; end > 0; --end) {
                auto t = first[l]; first[l] = first[l + end]; first[l + end] = t;
                heapify(l, end, 0);
            }
        };
        auto log2n = [&](USize v) noexcept {
            USize d = 0; while (v > 1) { v >>= 1; ++d; } return d;
        };
        struct Frame { USize l; USize r; USize depth; };
        Frame stack[64]; USize top = 0; stack[top++] = Frame{0, n - 1, log2n(n) * 2};
        constexpr USize kInsertionThreshold = 32;
        while (top) {
            auto f = stack[--top];
            USize l = f.l, r = f.r;
            if (l >= r) continue;
            if (r - l + 1 <= kInsertionThreshold) { insertion_sort(l, r); continue; }
            if (f.depth == 0) { heap_sort(l, r); continue; }
            USize p = partition(l, r);
            if (p > 0) {
                if (p - 1 > l) stack[top++] = Frame{ l, p - 1, f.depth - 1 };
            }
            if (p + 1 < r) stack[top++] = Frame{ p + 1, r, f.depth - 1 };
        }
    }

    template<typename R>
    requires Range<R>
    void Sort(R&& r) noexcept { Sort(r.begin(), r.end()); }

    template<typename It>
    void Sort(ExecutionPolicy policy, It first, It last) noexcept { (void)policy; Sort(first, last); }

    template<typename R>
    requires Range<R>
    void Sort(ExecutionPolicy policy, R&& r) noexcept { Sort(policy, r.begin(), r.end()); }
}
