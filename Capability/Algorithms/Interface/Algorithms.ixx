export module Algorithms;

import Language;

export namespace Algorithms {
    template<typename It>
    void Sort(It first, It last) noexcept {
        if (first == last) return;
        auto n = static_cast<USize>(last - first);
        auto part = [&](USize l, USize r) noexcept -> USize {
            auto pivot = first[r];
            USize i = l;
            for (USize j = l; j < r; ++j) { if (first[j] < pivot) { auto tmp = first[i]; first[i] = first[j]; first[j] = tmp; ++i; } }
            auto tmp = first[i]; first[i] = first[r]; first[r] = tmp; return i;
        };
        struct Frame { USize l; USize r; };
        Frame stack[64]; USize top = 0; stack[top++] = Frame{0, n - 1};
        while (top) { auto f = stack[--top]; if (f.l >= f.r) continue; USize p = part(f.l, f.r);
            if (p > 0 && p - 1 > f.l) stack[top++] = Frame{ f.l, p - 1 };
            if (p + 1 < f.r)        stack[top++] = Frame{ p + 1, f.r };
        }
    }

    template<typename R>
    requires Range<R>
    void Sort(R&& r) noexcept { Sort(r.begin(), r.end()); }

    template<typename R, typename Pred>
    requires Range<R>
    [[nodiscard]] auto Find(R&& r, Pred pred) noexcept {
        for (auto it = r.begin(); it != r.end(); ++it) { if (pred(*it)) return it; } return r.end();
    }

    template<typename R, typename Pred>
    requires Range<R>
    [[nodiscard]] USize RemoveIf(R&& r, Pred pred) noexcept {
        auto it = r.begin(); auto last = r.end(); USize w = 0; USize i = 0;
        for (; it != last; ++it, ++i) { if (!pred(*it)) { ++w; } }
        return i - w;
    }
}