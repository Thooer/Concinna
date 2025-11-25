
export module Platform:Time;

import Prm;

export namespace Platform
{
    
    class Time
    {
    public:
        using TimePoint = Int64; // 从固定参考点开始的纳秒数
        using Duration  = Int64; // 时间间隔（单位：纳秒）

        // --- 预热一次性状态（例如高精度计时器频率缓存） ---
        static void Init() noexcept;

        // --- 获取当前时间点 ---
        [[nodiscard]] static TimePoint Now() noexcept;

        // --- 时间差计算（内联，零开销） ---
        [[nodiscard]] static constexpr Float64 DeltaSeconds(TimePoint start, TimePoint end) {
            return static_cast<Float64>(end - start) / 1'000'000'000.0;
        }
        [[nodiscard]] static constexpr Duration Delta(TimePoint start, TimePoint end) {
            return end - start;
        }

        // --- 延迟（休眠） ---
        static void SleepMs(UInt32 milliseconds) noexcept;      // 毫秒级休眠

        // --- 单位转换（内联，零开销） ---
        [[nodiscard]] static constexpr Float64 ToSeconds(Duration d) {
            return static_cast<Float64>(d) / 1'000'000'000.0;
        }
        [[nodiscard]] static constexpr Float64 ToMilliseconds(Duration d) {
            return static_cast<Float64>(d) / 1'000'000.0;
        }

        // 平台层不负责格式化，纯逻辑在 Time 模块
    };

    class WallClock
    {
    public:
        struct Data {
            UInt16 year;
            UInt8  month, day;
            UInt8  hour, minute, second;
            UInt16 millisecond;
        };

        [[nodiscard]] static Data Now() noexcept;

        [[nodiscard]] static Data UtcNow() noexcept;

        // 平台层不负责格式化，纯逻辑在 Time 模块
    };
}
