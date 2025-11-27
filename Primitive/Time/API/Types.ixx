export module Prm.Time:Types;
import Element;

export namespace Prm {
    using TimePoint = Int64;
    using Duration  = Int64;

    [[nodiscard]] constexpr Duration Delta(TimePoint start, TimePoint end);
    [[nodiscard]] constexpr Float64 DeltaSeconds(TimePoint start, TimePoint end);
    [[nodiscard]] constexpr Float64 ToSeconds(Duration d);
    [[nodiscard]] constexpr Float64 ToMilliseconds(Duration d);
}
