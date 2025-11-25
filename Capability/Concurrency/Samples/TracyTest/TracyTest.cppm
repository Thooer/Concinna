module;
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <random>
#include <iostream>
#include <cmath>

export module Concurrency.TracyTest;
import Language;
import Platform;
import System;
import Concurrency;
import Tools.Tracy;
namespace ConcurrencyTest {
    using namespace std::chrono_literals;

    // Job workload function with Tracy profiling
    void JobWorkload(void* data) noexcept 
    {
        Tools::Tracy::Zone z{"JobWorkload"};
        auto* counter = static_cast<std::atomic<int>*>(data);
        
        for (int i = 0; i < 1000; ++i) 
        {
            Tools::Tracy::Zone zi{"JobWorkIteration"};
            
            // Simulate some work
            volatile double result = 0.0;
            for (int j = 0; j < 100; ++j) 
            {
                result += std::sqrt(static_cast<double>(j));
            }
            
            counter->fetch_add(1, std::memory_order_relaxed);
            
            // Small delay
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }

    // Simple job execution test with Tracy profiling
    void JobExecutionTest() 
    {
        Tools::Tracy::Zone z{"JobExecutionTest"};
        Tools::Tracy::Message("Starting JobExecutionTest");
        
        constexpr int numJobs = 100;
        constexpr int numWorkers = 4;
        
        std::vector<std::atomic<int>> counters(numWorkers);
        auto& scheduler = Concurrency::Scheduler::Instance();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Submit jobs
        for (int i = 0; i < numJobs; ++i) 
        {
            int workerId = i % numWorkers;
            auto job = [&, workerId]() noexcept {
                JobWorkload(&counters[workerId]);
            };
            (void)scheduler.Run(std::move(job));
            
            if (i % 10 == 0) 
            {
                Tools::Tracy::Plot("JobsSubmitted", static_cast<double>(i));
            }
        }
        
        // Wait for completion
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        int totalOperations = 0;
        for (const auto& counter : counters) 
        {
            totalOperations += counter.load();
        }
        
        Tools::Tracy::Plot("TotalOperations", static_cast<double>(totalOperations));
        Tools::Tracy::Plot("JobExecutionTime_ms", static_cast<double>(duration.count()));
        Tools::Tracy::Plot("OperationsPerSecond", static_cast<double>(totalOperations) / (duration.count() / 1000.0));
        Tools::Tracy::Message("JobExecutionTest completed");
        
        std::cout << "JobExecutionTest: " << totalOperations << " operations in " 
                  << duration.count() << " ms\n";
    }

    // Manual parallel processing test with Tracy profiling
    void ParallelForTest() 
    {
        Tools::Tracy::Zone z{"ParallelForTest"};
        Tools::Tracy::Message("Starting ParallelForTest");
        
        constexpr size_t size = 1000000;
        std::vector<double> data(size, 1.0);
        std::vector<double> result(size);
        
        Tools::Tracy::Plot("DataSize", static_cast<double>(size));
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Use manual parallelization with Tracy instrumentation
        auto& scheduler = Concurrency::Scheduler::Instance();
        constexpr size_t chunkSize = 10000;
        constexpr size_t numChunks = (size + chunkSize - 1) / chunkSize;
        
        for (size_t chunk = 0; chunk < numChunks; ++chunk) 
        {
            auto chunkStart = chunk * chunkSize;
            auto chunkEnd = std::min(chunkStart + chunkSize, size);
            
            auto job = [chunkStart, chunkEnd, &data, &result]() noexcept {
                Tools::Tracy::Zone zk{"ParallelChunk"};
                for (size_t i = chunkStart; i < chunkEnd; ++i) 
                {
                    result[i] = data[i] * 2.0;
                }
            };
            (void)scheduler.Run(std::move(job));
        }
        
        // Wait for completion
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        Tools::Tracy::Plot("ParallelForTime_us", static_cast<double>(duration.count()));
        Tools::Tracy::Message("ParallelForTest completed");
        
        std::cout << "ParallelForTest: " << duration.count() << " microseconds\n";
    }

    // Stress test with Tracy profiling
    void StressTest() 
    {
        Tools::Tracy::Zone z{"StressTest"};
        Tools::Tracy::Message("Starting StressTest");
        
        constexpr int numJobs = 500;
        constexpr int numWorkers = 4;
        
        std::vector<std::atomic<int>> counters(numWorkers);
        auto& scheduler = Concurrency::Scheduler::Instance();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Submit jobs
        for (int i = 0; i < numJobs; ++i) 
        {
            int workerId = i % numWorkers;
            auto job = [&, workerId]() noexcept {
                JobWorkload(&counters[workerId]);
            };
            (void)scheduler.Run(std::move(job));
            
            if (i % 50 == 0) 
            {
                Tools::Tracy::Plot("JobsSubmitted", static_cast<double>(i));
            }
        }
        
        // Wait for completion
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        int totalOperations = 0;
        for (const auto& counter : counters) 
        {
            totalOperations += counter.load();
        }
        
        Tools::Tracy::Plot("TotalOperations", static_cast<double>(totalOperations));
        Tools::Tracy::Plot("StressTestTime_ms", static_cast<double>(duration.count()));
        Tools::Tracy::Plot("OperationsPerSecond", static_cast<double>(totalOperations) / (duration.count() / 1000.0));
        Tools::Tracy::Message("StressTest completed");
        
        std::cout << "StressTest: " << totalOperations << " operations in " 
                  << duration.count() << " ms\n";
    }

    // Memory stress test with Tracy profiling
    void MemoryStressTest() 
    {
        Tools::Tracy::Zone z{"MemoryStressTest"};
        Tools::Tracy::Message("Starting MemoryStressTest");
        auto heap = Platform::Memory::Heap::GetProcessDefault();
        
        std::vector<void*> allocations;
        allocations.reserve(1000);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Allocate memory
        for (int i = 0; i < 1000; ++i) 
        {
            Tools::Tracy::Zone za{"MemoryAllocation"};
            size_t size = 1024 + (i % 16) * 1024;
            auto r = Platform::Memory::Heap::AllocRaw(heap, size);
            void* ptr = r.IsOk() ? r.Value() : nullptr;
            
            if (ptr) 
            {
                allocations.push_back(ptr);
                Tools::Tracy::Plot("AllocatedSize", static_cast<double>(size));
            }
        }
        
        // Free memory
        for (void* ptr : allocations) 
        {
            Tools::Tracy::Zone zd{"MemoryDeallocation"};
            (void)Platform::Memory::Heap::FreeRaw(heap, ptr);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        Tools::Tracy::Plot("MemoryStressTime_us", static_cast<double>(duration.count()));
        Tools::Tracy::Message("MemoryStressTest completed");
        
        std::cout << "MemoryStressTest: " << allocations.size() 
                  << " allocations in " << duration.count() << " microseconds\n";
    }

}

export int main() {
    using namespace ConcurrencyTest;
        Tools::Tracy::SetThreadName("TracyTestMain");
        Tools::Tracy::Message("Starting TracyTest with Concurrency");
        
        // Initialize Concurrency scheduler
        auto& scheduler = Concurrency::Scheduler::Instance();
        (void)scheduler.Start(4); // Start with 4 workers
        
        // Get system info and plot it
        auto cpu = System::SystemInfo::Cpu();
        Tools::Tracy::Plot("CPUCores", static_cast<double>(cpu.physicalCores));
        Tools::Tracy::Plot("CPULogicalCores", static_cast<double>(cpu.logicalCores));
        
        std::cout << "System Info:\n";
        std::cout << "  CPU Logical Cores: " << cpu.logicalCores << "\n";
        std::cout << "  CPU Physical Cores: " << cpu.physicalCores << "\n";
        
        // Run tests
        std::cout << "\n=== Running Concurrency Tests with Tracy Profiling ===\n";
        
        try 
        {
            // Run each test with frame marks
            Tools::Tracy::FrameMarkStart("JobExecutionTest");
            JobExecutionTest();
            Tools::Tracy::FrameMarkEnd("JobExecutionTest");
            
            Tools::Tracy::FrameMarkStart("ParallelForTest");
            ParallelForTest();
            Tools::Tracy::FrameMarkEnd("ParallelForTest");
            
            Tools::Tracy::FrameMarkStart("StressTest");
            StressTest();
            Tools::Tracy::FrameMarkEnd("StressTest");
            
            Tools::Tracy::FrameMarkStart("MemoryStressTest");
            MemoryStressTest();
            Tools::Tracy::FrameMarkEnd("MemoryStressTest");
            
            Tools::Tracy::Message("All tests completed successfully");
            
        } 
        catch (const std::exception& e) 
        {
            Tools::Tracy::Message("Exception occurred during testing");
            std::cerr << "Exception: " << e.what() << "\n";
            return 1;
        }
        
        // Shutdown scheduler
        (void)scheduler.Stop();
        
        std::cout << "\n=== TracyTest completed ===\n";
        std::cout << "Check console output for Tracy messages (simplified implementation)\n";
        
        return 0;
}