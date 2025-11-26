import Prm.Time;
import <vector>;
import <cstdio>;
import <chrono>;


static double avgNs(const std::vector<double>& v) {
    double s = 0.0; for (double x : v) s += x; return v.empty() ? 0.0 : s / static_cast<double>(v.size());
}
static double stddevNs(const std::vector<double>& v, double mean) {
    if (v.empty()) return 0.0; double s = 0.0; for (double x : v) { double d = x - mean; s += d * d; } return std::sqrt(s / static_cast<double>(v.size()));
}

extern "C" int main() {
    
    Prm::ITimeSource* src = Prm::DefaultSource();

    const int Trials = 30;
    const int Iter = 1'000'000;

    std::vector<double> resSamples; resSamples.reserve(Trials);
    for (int t = 0; t < Trials; ++t) {
        Prm::TimePoint a = src->Now();
        Prm::TimePoint last = a;
        Int64 minDelta = 0;
        for (int i = 0; i < Iter; ++i) {
            Prm::TimePoint n = src->Now();
            Int64 d = n - last; if (d > 0 && (minDelta == 0 || d < minDelta)) minDelta = d; last = n;
        }
        resSamples.push_back(static_cast<double>(minDelta));
    }
    double resMean = avgNs(resSamples);
    double resStd = stddevNs(resSamples, resMean);
    std::printf("Resolution(ns) avg=%.2f std=%.2f\n", resMean, resStd);

    std::vector<double> nowCost; nowCost.reserve(Trials);
    for (int t = 0; t < Trials; ++t) {
        Prm::TimePoint s0 = src->Now();
        for (int i = 0; i < Iter; ++i) { (void)src->Now(); }
        Prm::TimePoint s1 = src->Now();
        double ns = static_cast<double>(s1 - s0);
        double per = ns / static_cast<double>(Iter);
        nowCost.push_back(per);
    }
    double nowMean = avgNs(nowCost);
    double nowStd = stddevNs(nowCost, nowMean);
    std::printf("Now cost(ns/op) avg=%.2f std=%.2f\n", nowMean, nowStd);

    struct SleepCase { Int64 ns; const char* name; };
    std::vector<SleepCase> cases = { {1'000ll, "1us"}, {10'000ll, "10us"}, {100'000ll, "100us"}, {1'000'000ll, "1ms"}, {5'000'000ll, "5ms"}, {10'000'000ll, "10ms"} };
    for (auto c : cases) {
        std::vector<double> errs; errs.reserve(Trials);
        for (int t = 0; t < Trials; ++t) {
            Prm::TimePoint s0 = src->Now();
            Prm::SleepPreciseNs(c.ns);
            Prm::TimePoint s1 = src->Now();
            double actual = static_cast<double>(s1 - s0);
            errs.push_back(actual - static_cast<double>(c.ns));
        }
        double m = avgNs(errs);
        double sd = stddevNs(errs, m);
        std::printf("Sleep(%s) error(ns) avg=%.2f std=%.2f\n", c.name, m, sd);
    }

    Prm::Stopwatch sw{}; sw.src = src; sw.Start(); Prm::SleepPreciseNs(10'000'000ll); sw.Stop();
    double swMs = static_cast<double>(sw.Elapsed()) / 1'000'000.0;
    std::printf("Stopwatch 10ms measured=%.2f ms\n", swMs);

    Prm::Timer tm{}; tm.src = src; tm.Start(); Prm::SleepPreciseNs(5'000'000ll); double lap = static_cast<double>(tm.Lap()) / 1'000'000.0; Prm::SleepPreciseNs(7'000'000ll); double el = static_cast<double>(tm.Elapsed()) / 1'000'000.0;
    std::printf("Timer Lap=%.2f ms Elapsed=%.2f ms\n", lap, el);

    return 0;
}
