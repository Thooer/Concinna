import Language;
import Platform;
import Memory;
import Concurrency;
import <cstdio>;

struct SpawnPack { int level; int depth; int bf; Concurrency::Counter* c; };

static void SpawnThunk(void* p) noexcept {
    auto* sp = static_cast<SpawnPack*>(p);
    if (sp->level < sp->depth) {
        sp->c->Add(static_cast<UInt32>(sp->bf));
        for (int i = 0; i < sp->bf; ++i) {
            auto r = Platform::Memory::Heap::AllocRaw(Platform::Memory::Heap::GetProcessDefault(), sizeof(SpawnPack));
            if (!r.IsOk()) continue;
            auto* child = static_cast<SpawnPack*>(r.Value());
            child->level = sp->level + 1; child->depth = sp->depth; child->bf = sp->bf; child->c = sp->c;
            Concurrency::Job cj{}; cj.invoke = &SpawnThunk; cj.arg = child;
            (void)Concurrency::RunWithCounter(cj, *sp->c);
        }
    }
    (void)Platform::Memory::Heap::FreeRaw(Platform::Memory::Heap::GetProcessDefault(), sp);
}

static void NopThunk(void*) noexcept {}

extern "C" int main() {
    auto& sch = Concurrency::Scheduler::Instance();
    (void)sch.Start(0u);
    auto h = Platform::Memory::Heap::GetProcessDefault();

    auto start = Platform::Time::Now();
    Concurrency::Job j{}; j.invoke = &NopThunk; j.arg = nullptr; (void)sch.Submit(j);
    Platform::ThreadSleepMs(50);
    auto end = Platform::Time::Now();
    auto dt = Platform::Time::DeltaSeconds(start, end);
    printf("TreeExplosion dt=%.6f s\n", dt);
    (void)sch.Stop();
    return 0;
}
