import Language;
import Containers;

extern "C" int main() {
    using namespace Containers;
    HashMap<UInt32, UInt32> m{};
    auto s1 = m.Put(1u, 10u);
    if (!s1.Ok()) return 1;
    auto s2 = m.Put(2u, 20u);
    if (!s2.Ok()) return 2;
    auto p = m.GetPtr(2u);
    if (!p || *p != 20u) return 3;
    bool er = m.Erase(1u);
    if (!er) return 4;
    auto p2 = m.GetPtr(1u);
    if (p2) return 5;
    return 0;
}