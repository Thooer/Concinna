import Language;
import Platform;
import Concurrency;

export module Concurrency.BatchSubmitDemo;

static void BatchEntry(void* p) noexcept {
    auto* c = static_cast<Concurrency::Counter*>(p);
    c->SignalComplete();
}

export int main() {
    auto& sch = Concurrency::Scheduler::Instance();
    (void)sch.Start(4);

    Concurrency::Counter c{};
    c.Reset(8);

    Concurrency::Job jobs[8];
    for (int i = 0; i < 8; ++i) {
        Concurrency::Job j{}; j.invoke = &BatchEntry; j.arg = &c;
        if ((i % 2) == 0) { j.qos = static_cast<UInt8>(Concurrency::QoS::High); }
        else { j.qos = static_cast<UInt8>(Concurrency::QoS::Normal); }
        jobs[i] = j;
    }

    (void)sch.SubmitBatch(jobs, 8);
    Concurrency::WaitForCounter(c);
    (void)sch.Stop();
    return 0;
}