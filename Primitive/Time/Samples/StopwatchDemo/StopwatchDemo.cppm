import Prm.Time;

struct LocalTimeSource : Prm::ITimeSource {
    Prm::TimePoint Now() noexcept override { return Prm::Now(); }
};

extern "C" int main() {
    LocalTimeSource src{};
    Prm::Stopwatch sw{};
    sw.src = &src;
    sw.Start();
    Prm::SleepMs(10);
    sw.Stop();
    auto d = sw.Elapsed();
    Char8 buf[32]{};
    auto r = Prm::FormatDuration(d, Span<Char8, DynamicExtent>{buf, 32});
    if (!r.IsOk()) return 1;
    return 0;
}
