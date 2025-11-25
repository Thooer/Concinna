import Language;
import Memory;
import Platform;
import <vector>;
import <chrono>;
import <random>;

extern "C" int main() {
    using namespace Memory;
    using namespace Platform;
    
    
    // 测试参数配置
    constexpr USize BlockSize = 32;
    constexpr USize Capacity = 1u << 20;
    constexpr USize TestCount = 1000;
    
    // 创建PoolAllocatorResource
    PoolAllocatorResource allocator{BlockSize, Capacity};
    
    // 原子变量
    Atomic<USize> totalAllocations{0};
    Atomic<USize> totalDeallocations{0};
    Atomic<USize> liveMemoryBlocks{0};
    Atomic<USize> testErrors{0};
    
    // 简单分配释放测试
    std::vector<void*> allocations;
    allocations.reserve(TestCount);
    
    // 1. 分配测试
    for (USize i = 0; i < TestCount; ++i) {
        auto result = allocator.Allocate(BlockSize, Alignment::Default);
        if (result.IsOk()) {
            auto block = result.Value();
            void* ptr = block.ptr;
            
            // 写入数据验证内存完整性
            *static_cast<volatile UInt32*>(ptr) = 0xDEADBEEF;
            
            allocations.push_back(ptr);
            totalAllocations.FetchAdd(1, MemoryOrder::Relaxed);
            liveMemoryBlocks.FetchAdd(1, MemoryOrder::Relaxed);
        } else {
            testErrors.FetchAdd(1, MemoryOrder::Relaxed);
        }
    }
    
    // 2. 释放测试
    for (void* ptr : allocations) {
        if (ptr != nullptr) {
            // 验证内存没有被破坏
            volatile UInt32 value = *static_cast<volatile UInt32*>(ptr);
            if (value != 0xDEADBEEF) {
                testErrors.FetchAdd(1, MemoryOrder::Relaxed);
            }
            
            // 释放内存
            allocator.Deallocate(MemoryBlock{ptr, BlockSize}, Alignment::Default);
            totalDeallocations.FetchAdd(1, MemoryOrder::Relaxed);
            liveMemoryBlocks.FetchSub(1, MemoryOrder::Relaxed);
        }
    }
    
    // 3. 清理
    allocator.FlushLocalToRemote();
    
    // 获取分配器统计信息
    auto stats = allocator.GetStats();
    
    // 验证测试结果
    bool testPassed = true;
    
    // 1. 内存泄漏检查
    if (stats.currentUsage != 0) {
        testPassed = false;
    }
    
    // 2. 统计一致性检查
    if (stats.allocationCount != totalAllocations.Load(MemoryOrder::Acquire)) {
        testPassed = false;
    }
    
    if (stats.freeCount != totalDeallocations.Load(MemoryOrder::Acquire)) {
        testPassed = false;
    }
    
    // 3. 实时内存块检查
    if (liveMemoryBlocks.Load(MemoryOrder::Acquire) != 0) {
        testPassed = false;
    }
    
    // 4. 错误检查
    if (testErrors.Load(MemoryOrder::Acquire) > 0) {
        testPassed = false;
    }
    
    // 调试模式额外验证
    #if defined(CONFIG_DEBUG) || defined(_DEBUG)
    {
        allocator.DumpStats();
        
        static_assert(sizeof(PoolAllocatorResource::FreeNode) == sizeof(void*), 
                      "FreeNode结构体大小异常");
        
        static_assert(AtomicIsAlwaysLockFree<PoolAllocatorResource::FreeNode*>(),
                      "Remote head原子操作不是无锁的");
        
        if (stats.totalAllocated != stats.totalFreed) {
            testPassed = false;
        }
    }
    #endif
    
    // 性能统计输出
    DebugBreak();
    
    return testPassed ? 0 : 1;
}