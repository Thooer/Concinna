module Concurrency;
import Language;
import :Job;

namespace Concurrency {
    void Job::Run() noexcept { if (invoke) invoke(arg); }
}