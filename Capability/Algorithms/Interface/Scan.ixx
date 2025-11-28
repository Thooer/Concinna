export module Cap.Algorithms:Scan;

import Language;
import :Policy;

export namespace Cap {
    template<typename T, typename Op>
    T Reduce(ExecutionPolicy policy, Span<const T> input, T init, Op op) noexcept {
        T acc = init;
        for (USize i = 0; i < input.size(); ++i) { acc = op(acc, input[i]); }
        return acc;
    }

    template<typename T>
    void ExclusiveScan(ExecutionPolicy policy, Span<const T> input, Span<T> output, T init) noexcept {
        USize n = input.size();
        if (output.size() < n) { n = output.size(); }
        T acc = init;
        for (USize i = 0; i < n; ++i) { output[i] = acc; acc = acc + input[i]; }
    }
}
