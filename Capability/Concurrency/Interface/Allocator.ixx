export module Cap.Concurrency:Allocator;

import Language;
import Memory;

export namespace Cap {
    [[nodiscard]] Cap::Allocator CurrentAllocator() noexcept;
}
