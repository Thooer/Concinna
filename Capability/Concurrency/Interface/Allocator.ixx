export module Concurrency:Allocator;

import Language;
import Memory;
import :Scheduler;
import :Fiber;

export namespace Concurrency {
    [[nodiscard]] Memory::Allocator CurrentAllocator() noexcept;
}