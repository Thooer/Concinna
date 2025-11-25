#include <cstddef>
import Engine.Resource.MainlineBDataMemory;

int main() {
    bool ok = Nova::Samples::MainlineBDataMemory::Run();
    return ok ? 0 : 1;
}