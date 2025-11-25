module;
module Prm.Time;

import :Time;

namespace Prm {
    

    void Init() noexcept {}
    TimePoint Now() noexcept { return 0; }
    void SleepMs(UInt32) noexcept {}
    
    

    WallClock::Data WallClock::Now() noexcept { return WallClock::Data{0,0,0,0,0,0,0}; }
    WallClock::Data WallClock::UtcNow() noexcept { return WallClock::Data{0,0,0,0,0,0,0}; }
    
}
