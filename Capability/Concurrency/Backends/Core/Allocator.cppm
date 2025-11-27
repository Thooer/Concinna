module Cap.Concurrency;
import Language;
import Memory;
import :Allocator;

namespace Cap {
    [[nodiscard]] Cap::Allocator CurrentAllocator() noexcept {
        return Cap::Allocator();
    }
}
