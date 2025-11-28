# Algorithms 模块

## 1. 模块定位

* **职责**：提供基础算法和并行算法实现，为引擎其他模块提供高效的算法支持。
* **边界**：专注于算法实现，不涉及具体业务逻辑；仅依赖Language、Containers、Memory和Concurrency模块。
* **外部依赖**：
  - Language：语言基础支持
  - Containers：容器支持
  - Memory：内存管理支持
  - Concurrency：并发支持
* **核心设计原则**：
  - 高性能优先
  - 接口简洁易用
  - 支持并行执行
  - 模板化设计，支持多种类型
  - 范围-based接口，方便直接操作容器

## 2. 设计

* **模块结构**：
  - Interface/：算法接口定义
  - Sample/：测试示例
  - CMakeLists.txt：构建配置
  - README.md：模块文档

* **依赖关系**：
  ```
  Algorithms
  ├── Language
  ├── Containers
  ├── Memory
  └── Concurrency
  ```

## 3. API

### 3.1 排序算法

#### 3.1.1 Sort
```cpp
// 对迭代器范围内的元素进行排序
// 复杂度：O(n log n)
template<typename It>
void Sort(It first, It last) noexcept;

// 对范围对象进行排序
// 复杂度：O(n log n)
template<typename R>
requires Range<R>
void Sort(R&& r) noexcept;
```

### 3.2 查找算法

#### 3.2.1 Find
```cpp
// 在范围中查找满足条件的元素
// 返回：指向第一个满足条件的元素的迭代器，若未找到则返回end()
template<typename R, typename Pred>
requires Range<R>
[[nodiscard]] auto Find(R&& r, Pred pred) noexcept;
```

### 3.3 移除算法

#### 3.3.1 RemoveIf
```cpp
// 移除范围中满足条件的元素
// 返回：被移除的元素数量
template<typename R, typename Pred>
requires Range<R>
[[nodiscard]] USize RemoveIf(R&& r, Pred pred) noexcept;
```

### 3.4 填充算法

#### 3.4.1 Fill
```cpp
// 用指定值填充迭代器范围内的元素
template<typename It, typename T>
void Fill(It first, It last, const T& value) noexcept;

// 用指定值填充范围对象
template<typename R, typename T>
requires Range<R>
void Fill(R&& r, const T& value) noexcept;
```

### 3.5 复制算法

#### 3.5.1 Copy
```cpp
// 将迭代器范围内的元素复制到目标位置
// 返回：指向目标范围末尾的迭代器
template<typename InIt, typename OutIt>
OutIt Copy(InIt first, InIt last, OutIt d_first) noexcept;

// 将范围对象的元素复制到目标位置
// 返回：指向目标范围末尾的迭代器
template<typename R, typename OutIt>
requires Range<R>
OutIt Copy(R&& r, OutIt d_first) noexcept;
```

### 3.6 并行算法

#### 3.6.1 ParallelFor
```cpp
// 并行执行for循环
// begin：起始索引
// end：结束索引（不包含）
// grain：任务粒度，控制并行度
// func：每个索引执行的函数
template<typename Index, typename Func>
requires (IsIntegral<Index> && IsInvocable<Func, Index>)
void ParallelFor(Index begin, Index end, Index grain, Func func) noexcept;
```

#### 3.6.2 ParallelForEach
```cpp
// 并行遍历范围对象
// r：要遍历的范围
// func：每个元素执行的函数
template<typename R, typename Func>
requires Range<R> && IsInvocable<Func, decltype(*std::declval<R>().begin())>
void ParallelForEach(R&& r, Func func) noexcept;
```

## 4. Samples

### 4.1 AlgorithmsTest

**测试内容**：
- 测试Sort算法的正确性
- 测试Find算法的正确性
- 测试Fill算法的正确性
- 测试Copy算法的正确性
- 测试ParallelFor算法的正确性

**测试结果**：
- 所有算法均能正确执行
- 并行算法能够有效利用多核CPU
- 算法性能满足预期

**使用示例**：
```cpp
// 测试Sort算法
int arr[] = {5, 2, 9, 1, 5, 6};
Algorithms::Sort(arr, arr + 6);

// 测试ParallelFor算法
constexpr int size = 1000;
auto vec = Containers::Vector<int>(size, 0);
Algorithms::ParallelFor(0, size, 100, [&vec](int i) {
    vec[i] = i * i;
});
```