export module Prm.Sync:Backoff;
import Prm.Element;
import :Fence;
import <thread>;

export namespace Prm {
    struct Backoff {
        UInt32 n{0};
        inline void Reset() noexcept { n = 0; }
        inline void Pause() noexcept { CpuRelax(); }
        inline void Yield() noexcept { std::this_thread::yield(); }
        inline void Next() noexcept {
            if (n < 16u) { Pause(); }
            else if (n < 64u) { Pause(); Yield(); }
            else { Yield(); }
            ++n;
        }
    };
}

