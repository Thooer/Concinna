module Prm.Time;
import Prm.Platform;
import :Time;

namespace Prm {
    constexpr Duration Delta(TimePoint start, TimePoint end) {
        return end - start;
    }
    constexpr Float64 DeltaSeconds(TimePoint start, TimePoint end) {
        return static_cast<Float64>(end - start) / 1'000'000'000.0;
    }
    constexpr Float64 ToSeconds(Duration d) {
        return static_cast<Float64>(d) / 1'000'000'000.0;
    }
    constexpr Float64 ToMilliseconds(Duration d) {
        return static_cast<Float64>(d) / 1'000'000.0;
    }

    void Stopwatch::Start() noexcept {
        if (running || src == nullptr) return;
        t0 = src->Now();
        running = true;
    }
    void Stopwatch::Stop() noexcept {
        if (!running || src == nullptr) return;
        acc += (src->Now() - t0);
        running = false;
    }
    void Stopwatch::Reset() noexcept {
        running = false; acc = 0; t0 = 0;
    }
    Duration Stopwatch::Elapsed() const noexcept {
        if (!src) return acc;
        return running ? acc + (src->Now() - t0) : acc;
    }

    Expect<USize> FormatDuration(Duration d, Span<Char8, DynamicExtent> buffer) noexcept {
        const Int64 totalMs = d / 1'000'000ll;
        const Int64 ms = totalMs % 1000ll;
        const Int64 totalSec = totalMs / 1000ll;
        const Int64 sec = totalSec % 60ll;
        const Int64 totalMin = totalSec / 60ll;
        const Int64 min = totalMin % 60ll;
        const Int64 hour = totalMin / 60ll;
        const USize needed = 12u;
        if (buffer.size() < needed) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
        }
        auto put2 = [](Char8* p, Int64 v) {
            p[0] = static_cast<Char8>('0' + ((v / 10) % 10));
            p[1] = static_cast<Char8>('0' + (v % 10));
        };
        auto put3 = [](Char8* p, Int64 v) {
            p[0] = static_cast<Char8>('0' + ((v / 100) % 10));
            p[1] = static_cast<Char8>('0' + ((v / 10) % 10));
            p[2] = static_cast<Char8>('0' + (v % 10));
        };
        put2(buffer.data(), hour % 100);
        buffer[2] = ':';
        put2(buffer.data() + 3, min % 100);
        buffer[5] = ':';
        put2(buffer.data() + 6, sec % 100);
        buffer[8] = '.';
        put3(buffer.data() + 9, ms % 1000);
        return Expect<USize>::Ok(12u);
    }

    Expect<USize> WallClock::Format(const WallClock::Data& wc, Span<Char8, DynamicExtent> buffer) noexcept {
        const USize needed = 23u;
        if (buffer.size() < needed) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::OutOfRange));
        }
        Char8* out = buffer.data();
        const UInt16 y = wc.year;
        out[0] = static_cast<Char8>('0' + ((y / 1000) % 10));
        out[1] = static_cast<Char8>('0' + ((y / 100) % 10));
        out[2] = static_cast<Char8>('0' + ((y / 10) % 10));
        out[3] = static_cast<Char8>('0' + (y % 10));
        out[4] = '-';
        const UInt16 mo = wc.month;
        out[5] = static_cast<Char8>('0' + ((mo / 10) % 10));
        out[6] = static_cast<Char8>('0' + (mo % 10));
        out[7] = '-';
        const UInt16 d = wc.day;
        out[8] = static_cast<Char8>('0' + ((d / 10) % 10));
        out[9] = static_cast<Char8>('0' + (d % 10));
        out[10] = ' ';
        const UInt16 h = wc.hour;
        out[11] = static_cast<Char8>('0' + ((h / 10) % 10));
        out[12] = static_cast<Char8>('0' + (h % 10));
        out[13] = ':';
        const UInt16 mi = wc.minute;
        out[14] = static_cast<Char8>('0' + ((mi / 10) % 10));
        out[15] = static_cast<Char8>('0' + (mi % 10));
        out[16] = ':';
        const UInt16 s = wc.second;
        out[17] = static_cast<Char8>('0' + ((s / 10) % 10));
        out[18] = static_cast<Char8>('0' + (s % 10));
        out[19] = '.';
        const UInt16 ms = wc.millisecond;
        out[20] = static_cast<Char8>('0' + ((ms / 100) % 10));
        out[21] = static_cast<Char8>('0' + ((ms / 10) % 10));
        out[22] = static_cast<Char8>('0' + (ms % 10));
        return Expect<USize>::Ok(23u);
    }

    void Timer::Start() noexcept {
        if (running || src == nullptr) return;
        t0 = src->Now();
        lap0 = t0;
        running = true;
    }
    void Timer::Stop() noexcept {
        running = false;
    }
    void Timer::Reset() noexcept {
        running = false; t0 = 0; lap0 = 0;
    }
    Duration Timer::Elapsed() const noexcept {
        if (!src || !running) return 0;
        return src->Now() - t0;
    }
    Duration Timer::Restart() noexcept {
        if (!src) return 0;
        const auto now = src->Now();
        Duration d = running ? (now - t0) : 0;
        t0 = now;
        lap0 = now;
        running = true;
        return d;
    }
    Duration Timer::Lap() noexcept {
        if (!src || !running) return 0;
        const auto now = src->Now();
        Duration d = now - lap0;
        lap0 = now;
        return d;
    }

    static MonotonicSource g_defaultSrc{};
    ITimeSource* DefaultSource() noexcept { return &g_defaultSrc; }

    TimePoint MonotonicSource::Now() noexcept {
        const PlatformAPI* api = GetPlatformAPI();
        return api ? api->time.Now() : 0;
    }

    void SleepPreciseNs(Int64 ns) noexcept {
        if (ns <= 0) return;
        const Int64 coarse = ns - 200'000ll;
        const PlatformAPI* api = GetPlatformAPI();
        if (coarse > 0 && api) api->time.SleepMs(static_cast<UInt32>(coarse / 1'000'000ll));
        const auto start = MonotonicSource::Now();
        while (Delta(start, MonotonicSource::Now()) < (ns - (coarse > 0 ? coarse : 0))) {
        }
    }
}
