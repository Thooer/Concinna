export module Cap.Algorithms:Primitives;

import Language;
import Element;
import :Policy;

export namespace Cap {
    template<typename It, typename T>
    void Fill(It first, It last, const T& value) noexcept { for (; first != last; ++first) { *first = value; } }

    template<typename R, typename T>
    requires Range<R>
    void Fill(R&& r, const T& value) noexcept { Fill(r.begin(), r.end(), value); }

    template<typename InIt, typename OutIt>
    OutIt Copy(InIt first, InIt last, OutIt d_first) noexcept { for (; first != last; ++first, ++d_first) { *d_first = *first; } return d_first; }

    template<typename R, typename OutIt>
    requires Range<R>
    OutIt Copy(R&& r, OutIt d_first) noexcept { return Copy(r.begin(), r.end(), d_first); }

    template<typename R, typename Pred>
    requires Range<R>
    [[nodiscard]] auto Find(R&& r, Pred pred) noexcept { for (auto it = r.begin(); it != r.end(); ++it) { if (pred(*it)) return it; } return r.end(); }

    template<typename Index, typename Func>
    requires (Integral<Index> && Invocable<Func, Index>)
    void ParallelFor(Index begin, Index end, Index grain, Func func) noexcept { if (begin >= end) return; for (Index i = begin; i < end; ++i) { func(i); } }

    template<typename R, typename Func>
    requires Range<R>
    void ParallelForEach(R&& r, Func func) noexcept { for (auto it = r.begin(); it != r.end(); ++it) { func(*it); } }

    template<typename T, typename Func>
    void ForEach(ExecutionPolicy policy, Span<T> range, Func func) noexcept {
        USize n = range.size();
        if (policy.mode == ExecutionPolicy::Mode::Sequential || n < static_cast<USize>(4096)) {
            for (USize i = 0; i < n; ++i) { func(range[i]); }
        } else {
            for (USize i = 0; i < n; ++i) { func(range[i]); }
        }
    }

    template<typename T>
    void Fill(Span<T> dst, const T& value) noexcept { for (USize i=0;i<dst.size();++i) { dst[i]=value; } }

    template<typename T>
    void Copy(Span<const T> src, Span<T> dst) noexcept {
        USize n = src.size(); if (dst.size()<n) n=dst.size();
        if constexpr (TriviallyCopyable<T>) {
            if (n > 0) { MemMove(dst.data(), src.data(), n * sizeof(T)); }
        } else {
            for (USize i=0;i<n;++i){ dst[i]=src[i]; }
        }
    }

    template<typename T, typename Predicate>
    T* FindIf(Span<T> range, Predicate pred) noexcept { for (USize i=0;i<range.size();++i){ if (pred(range[i])) return &range[i]; } return nullptr; }
}
