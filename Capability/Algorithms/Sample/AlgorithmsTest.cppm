export module AlgorithmsTest;

import Lang;
import Cap.Algorithms;
import <cstdio>;

// 测试排序算法
export void TestSort() noexcept {
    // 测试基础排序
    int arr[] = {5, 2, 9, 1, 5, 6};
    Cap::Sort(Cap::Seq, arr, arr + 6);
    
    // 验证排序结果
    std::printf("Sort basic: %d %d %d %d %d %d\n", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
    
    // 测试范围版本的排序
    int vecArr[] = {5, 2, 9, 1, 5, 6};
    Cap::Sort(Cap::Seq, vecArr, vecArr + 6);
    
    // 验证排序结果
    std::printf("Sort range: %d %d %d %d %d %d\n", vecArr[0], vecArr[1], vecArr[2], vecArr[3], vecArr[4], vecArr[5]);
}

// 测试查找算法
export void TestFind() noexcept {
    int fArr[] = {5, 2, 9, 1, 5, 6};
    bool found9 = false; bool found10 = false;
    for (int i = 0; i < 6; ++i) { if (fArr[i] == 9) found9 = true; if (fArr[i] == 10) found10 = true; }
    std::printf("Find 9: %s\n", found9 ? "found" : "not found");
    std::printf("Find 10: %s\n", found10 ? "found" : "not found");
}

// 测试填充算法
export void TestFill() noexcept {
    int fillArr[] = {5, 2, 9, 1, 5, 6};
    Cap::Fill(fillArr, fillArr + 6, 42);
    std::printf("Fill: %d %d %d %d %d %d\n", fillArr[0], fillArr[1], fillArr[2], fillArr[3], fillArr[4], fillArr[5]);
}

// 测试复制算法
export void TestCopy() noexcept {
    int srcArr[] = {5, 2, 9, 1, 5, 6};
    int destArr[6] = {};
    Cap::Copy(srcArr, srcArr + 6, destArr);
    std::printf("Copy: %d %d %d %d %d %d\n", destArr[0], destArr[1], destArr[2], destArr[3], destArr[4], destArr[5]);
}

// 测试并行For算法
export void TestParallelFor() noexcept {
    constexpr int size = 1000;
    int vec[size] = {};
    
    // 使用并行For填充值
    Cap::ParallelFor(0, size, 100, [&vec](int i) { vec[i] = i * i; });
    
    // 验证结果
    std::printf("ParallelFor: vec[0]=%d vec[10]=%d vec[999]=%d\n", vec[0], vec[10], vec[999]);
}

export void TestReduce() noexcept {
    int a[6] = {1,2,3,4,5,6};
    auto s = Cap::Reduce(Cap::Seq, Span<const int>{a, 6}, 0, [](int acc, int v){ return acc + v; });
    std::printf("Reduce: %d\n", s);
}

export void TestExclusiveScan() noexcept {
    int a[4] = {1,2,3,4};
    int out[4] = {};
    Cap::ExclusiveScan(Cap::Seq, Span<const int>{a, 4}, Span<int>{out, 4}, 0);
    std::printf("ExclusiveScan: %d %d %d %d\n", out[0], out[1], out[2], out[3]);
}

export void TestRadixSort() noexcept {
    UInt32 keys[8] = { 9u, 1u, 0u, 7u, 3u, 2u, 8u, 5u };
    UInt32 values[8] = { 90u, 10u, 0u, 70u, 30u, 20u, 80u, 50u };
    Cap::RadixSort(Span<UInt32>{keys, 8});
    std::printf("Radix keys: %u %u %u %u %u %u %u %u\n", keys[0], keys[1], keys[2], keys[3], keys[4], keys[5], keys[6], keys[7]);
    Cap::RadixSortPairs(Span<UInt32>{keys, 8}, Span<UInt32>{values, 8});
    std::printf("Radix pairs: %u %u %u %u %u %u %u %u | %u %u %u %u %u %u %u %u\n",
        keys[0], keys[1], keys[2], keys[3], keys[4], keys[5], keys[6], keys[7],
        values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7]);
}
