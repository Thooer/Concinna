import Lang;
import Memory;
import Containers;

extern "C" int main() {
    using namespace Containers;
    Vector<Int32> v{};
    for (Int32 i = 0; i < 100; ++i) {
        auto s = v.PushBack(i);
        if (!s.Ok()) return 1;
    }
    if (v.Size() != 100) return 2;
    for (Int32 i = 0; i < 100; ++i) {
        if (v[i] != i) return 3;
    }
    v.Clear();
    if (v.Size() != 0) return 4;
    return 0;
}