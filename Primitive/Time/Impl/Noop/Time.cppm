module;
module Platform;

import Prm;
import :Time;

namespace Platform {
    

    void Time::Init() noexcept {}
    Time::TimePoint Time::Now() noexcept { return 0; }
    void Time::SleepMs(UInt32) noexcept {}
    
    

    WallClock::Data WallClock::Now() noexcept { return WallClock::Data{0,0,0,0,0,0,0}; }
    WallClock::Data WallClock::UtcNow() noexcept { return WallClock::Data{0,0,0,0,0,0,0}; }
    
}
