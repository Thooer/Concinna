import Language;
import Containers;

extern "C" int main() {
    using namespace Containers;
    Deque<UInt32> dq{};
    for (UInt32 i = 0; i < 16; ++i) { auto s = dq.PushBack(i); if (!s.Ok()) return 1; }
    for (UInt32 i = 0; i < 8; ++i) { dq.PopFront(); }
    for (UInt32 i = 0; i < 8; ++i) { auto s = dq.PushFront(100 + i); if (!s.Ok()) return 2; }
    if (dq.Size() != 16) return 3;
    if (dq[0] != 107) return 4;
    if (dq[15] != 15) return 5;
    return 0;
}