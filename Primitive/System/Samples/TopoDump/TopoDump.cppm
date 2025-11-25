import Prm.System;
import <cstdio>;

extern "C" int main() {
    auto topo = Prm::Detect();
    printf("Topo: logical=%u physical=%u tpc=%u numa=%u\n",
           topo.logicalCores, topo.physicalCores, topo.threadsPerCore, topo.numaNodes);
    auto cores = Prm::EnumerateCoreMasks();
    printf("Cores=%llu\n", (unsigned long long)cores.count);
    for (USize i = 0; i < cores.count && i < 8; ++i) {
        printf(" core[%llu]: group=%u mask=0x%llx\n", (unsigned long long)i, cores.data[i].group, (unsigned long long)cores.data[i].mask);
    }
    Prm::Release(cores);
    auto nodes = Prm::EnumerateNumaNodeMasks();
    printf("NUMA entries=%llu\n", (unsigned long long)nodes.count);
    for (USize i = 0; i < nodes.count && i < 8; ++i) {
        printf(" node[%llu]: node=%u group=%u mask=0x%llx\n", (unsigned long long)i, nodes.data[i].node, nodes.data[i].group, (unsigned long long)nodes.data[i].mask);
    }
    Prm::Release(nodes);
    auto caches = Prm::EnumerateCacheMasks();
    printf("Caches entries=%llu (show L3)\n", (unsigned long long)caches.count);
    USize shown = 0;
    for (USize i = 0; i < caches.count && shown < 8; ++i) {
        if (caches.data[i].level == 3u) {
            printf(" L3[%llu]: id=%u group=%u mask=0x%llx\n", (unsigned long long)i, caches.data[i].id, caches.data[i].group, (unsigned long long)caches.data[i].mask);
            ++shown;
        }
    }
    Prm::Release(caches);
    auto pkgs = Prm::EnumeratePackageMasks();
    printf("Packages entries=%llu\n", (unsigned long long)pkgs.count);
    for (USize i = 0; i < pkgs.count && i < 8; ++i) {
        printf(" pkg[%llu]: id=%u group=%u mask=0x%llx\n", (unsigned long long)i, pkgs.data[i].id, pkgs.data[i].group, (unsigned long long)pkgs.data[i].mask);
    }
    Prm::Release(pkgs);
    return 0;
}
