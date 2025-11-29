module Cap.Concurrency;
import Lang;
import Memory;
import :Allocator;

namespace Cap {
    [[nodiscard]] Cap::Allocator CurrentAllocator() noexcept {
        return Cap::Allocator();
    }
}
