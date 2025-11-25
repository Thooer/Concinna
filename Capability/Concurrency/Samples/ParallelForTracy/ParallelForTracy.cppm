export module Concurrency.ParallelForTracy;
import Language;
import Platform;
import Concurrency;
import Tools.Tracy;
export int main() {
    Tools::Tracy::SetThreadName("Main");
    auto& sch = Concurrency::Scheduler::Instance();
    (void)sch.Start(4);
    Tools::Tracy::Message("ParallelFor start");
    Tools::Tracy::FrameMark();
    constexpr int N = 100000;
    constexpr int G = 1000;
    auto st = Concurrency::ParallelFor(0, N, G, [](int) noexcept {
        Tools::Tracy::Zone z{"Task"};
        volatile double x = 0;
        for (int k = 0; k < 200; ++k) { x += k; }
    });
    (void)st;
    Tools::Tracy::FrameMark();
    Platform::ThreadSleepMs(100);
    (void)sch.Stop();
    return 0;
}