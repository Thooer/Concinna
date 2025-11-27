export module Prm.Time:Clock;
import Element;
import Flow;
import :Types;

export namespace Prm {
    struct ITimeSource {
        virtual ~ITimeSource() = default;
        virtual TimePoint Now() noexcept = 0;
    };

    struct MonotonicSource final : ITimeSource {
        TimePoint Now() noexcept override;
    };

    [[nodiscard]] ITimeSource* DefaultSource() noexcept;

    void Init() noexcept;
    [[nodiscard]] TimePoint Now() noexcept;
    void SleepMs(UInt32 milliseconds) noexcept;
    void SleepPreciseNs(Int64 ns) noexcept;
}
