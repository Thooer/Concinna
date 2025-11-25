import Prm;
import Time;
import Platform;

struct PlatformTimeSource : Time::ITimeSource {
    Time::TimePoint Now() noexcept override { return Platform::Time::Now(); }
};

extern "C" int main() {
    PlatformTimeSource src{};
    Time::Stopwatch sw{};
    sw.src = &src;
    sw.Start();
    Platform::Time::SleepMs(10);
    sw.Stop();
    auto d = sw.Elapsed();
    Char8 buf[32]{};
    auto r = Time::FormatDuration(d, Span<Char8, DynamicExtent>{buf, 32});
    if (!r.IsOk()) return 1;
    return 0;
}