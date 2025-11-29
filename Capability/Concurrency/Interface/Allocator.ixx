export module Cap.Concurrency:Allocator;

import Lang;
import Cap.Memory;

export namespace Cap {
    [[nodiscard]] Cap::Allocator CurrentAllocator() noexcept;
}
