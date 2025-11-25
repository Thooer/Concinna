import Prm.Time;
import Prm.Platform;

struct PlatformTimeSource : Prm::ITimeSource {
    Prm::TimePoint Now() noexcept override {
        const Prm::PlatformAPI* api = Prm::GetPlatformAPI();
        return api ? api->time.Now() : 0;
    }
};

extern "C" int main() {
    PlatformTimeSource src{};
    Prm::Stopwatch sw{};
    sw.src = &src;
    sw.Start();
    const Prm::PlatformAPI* api = Prm::GetPlatformAPI();
    if (api) api->time.SleepMs(10);
    sw.Stop();
    auto d = sw.Elapsed();
    Char8 buf[32]{};
    auto r = Prm::FormatDuration(d, Span<Char8, DynamicExtent>{buf, 32});
    if (!r.IsOk()) return 1;
    return 0;
}
