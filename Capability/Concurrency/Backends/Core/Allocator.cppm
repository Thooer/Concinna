module Concurrency;
import Language;
import Memory;
import :Allocator;
import :Scheduler;
import :Fiber;

namespace Concurrency {
    [[nodiscard]] Memory::Allocator CurrentAllocator() noexcept {
        auto* f = Scheduler::CurrentFiber();
        if (f) return Memory::Allocator(&f->frame);
        return Memory::Allocator();
    }
}