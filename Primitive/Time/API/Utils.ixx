export module Prm.Time:Utils;
import Lang.Element;
import Lang.Flow;
import :Types;
import :Clock;

export namespace Prm {
    struct Stopwatch {
        ITimeSource* src{nullptr};
        TimePoint t0{0};
        Duration acc{0};
        bool running{false};

        void Start() noexcept;
        void Stop() noexcept;
        void Reset() noexcept;
        [[nodiscard]] Duration Elapsed() const noexcept;
    };

    [[nodiscard]] Expect<USize> FormatDuration(Duration d, Span<Char8, DynamicExtent> buffer) noexcept;

    class WallClock {
    public:
        struct Data {
            UInt16 year;
            UInt8  month, day;
            UInt8  hour, minute, second;
            UInt16 millisecond;
        };

        [[nodiscard]] static Expect<USize>
        Format(const Data& wc, Span<Char8, DynamicExtent> buffer) noexcept;

        [[nodiscard]] static Data Now() noexcept;
        [[nodiscard]] static Data UtcNow() noexcept;
    };

    struct Timer {
        ITimeSource* src{nullptr};
        TimePoint t0{0};
        TimePoint lap0{0};
        bool running{false};

        void Start() noexcept;
        void Stop() noexcept;
        void Reset() noexcept;
        [[nodiscard]] Duration Elapsed() const noexcept;
        [[nodiscard]] Duration Restart() noexcept;
        [[nodiscard]] Duration Lap() noexcept;
    };
}
