export module Time;

import Prm;

export namespace Time {
    
    using TimePoint = Int64;
    using Duration  = Int64;

    [[nodiscard]] constexpr Duration Delta(TimePoint start, TimePoint end);
    [[nodiscard]] constexpr Float64 DeltaSeconds(TimePoint start, TimePoint end);
    [[nodiscard]] constexpr Float64 ToSeconds(Duration d);
    [[nodiscard]] constexpr Float64 ToMilliseconds(Duration d);

    struct ITimeSource {
        virtual ~ITimeSource() = default;
        virtual TimePoint Now() noexcept = 0;
    };

    struct MonotonicSource final : ITimeSource {
        TimePoint Now() noexcept override;
    };

    [[nodiscard]] ITimeSource* DefaultSource() noexcept;

    void SleepPreciseNs(Int64 ns) noexcept;

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

    [[nodiscard]] Expect<USize>
    FormatDuration(Duration d, Span<Char8, DynamicExtent> buffer) noexcept;

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