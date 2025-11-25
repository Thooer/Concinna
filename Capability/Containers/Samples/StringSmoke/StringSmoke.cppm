import Language;
import Containers;

extern "C" int main() {
    using namespace Containers;
    String s{};
    auto st1 = s.Assign(StringView{"hello"});
    if (!st1.Ok()) return 1;
    auto st2 = s.Append(StringView{" "});
    if (!st2.Ok()) return 2;
    auto st3 = s.Append(StringView{"world"});
    if (!st3.Ok()) return 3;
    if (s.Size() != 11) return 4;
    const auto* c = s.CStr();
    if (!c) return 5;
    if (c[11] != 0) return 6;
    if (c[0] != 'h' || c[10] != 'd') return 7;
    return 0;
}