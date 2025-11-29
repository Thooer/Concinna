module Concurrency;
import Lang;
import :Job;

namespace Concurrency {
    void Job::Run() noexcept { if (invoke) invoke(arg); }
}