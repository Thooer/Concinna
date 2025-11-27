# Semantics 模块

## 1. 模块定位

* **职责**：提供语义级别的编程支持，包括哈希算法、调试支持、生命周期管理和通用工具函数，为引擎提供统一的语义基础。
* **边界**：不提供复杂的业务逻辑，仅专注于语义级别的基础构建块；不依赖外部库，仅使用C++标准库、Element模块和Meta模块。
* **外部依赖**：C++标准库、Element模块、Meta模块。

## 2. 设计

### 2.1 核心设计原则

* **高性能**：所有组件设计优先考虑性能，避免不必要的运行时开销
* **编译时计算**：尽可能利用编译时计算，减少运行时成本
* **类型安全**：通过强类型设计确保语义操作的类型安全
* **可扩展性**：设计开放的语义框架，支持后续扩展

### 2.2 模块结构

* **Hash**：哈希算法实现，包括FNV-1a、黄金比例哈希、位混合等，以及哈希组合和范围哈希
* **Debug**：调试支持，包括断言、调试输出等
* **Lifetime**：生命周期管理，包括RAII支持、移动语义增强等
* **Utility**：通用工具函数，提供各种辅助功能

## 3. API

### 3.1 哈希算法

| 函数/类型 | 描述 |
|-----------|------|
| `Fnv1aBytes(data, size)` | FNV-1a哈希算法，用于字节序列 |
| `Fnv1aString(str)` | FNV-1a哈希算法，用于字符串 |
| `GoldenRatioHash(value)` | 黄金比例哈希，用于整数混合 |
| `CombineHashes(seed, hash)` | 组合多个哈希值 |
| `BitMixing(value)` | 位混合算法，用于指针哈希 |
| `SimpleStringHash(str)` | 简单字符串哈希，用于轻量级使用 |
| `HashValue(value)` | 获取值的哈希值的便捷函数 |
| `HashCombine(values...)` | 组合多个值的哈希值 |
| `HashRange(range)` | 计算范围/容器的哈希值 |
| `DefaultHasher<T>` | 容器的默认哈希器 |
| `Hashable` | 可哈希类型概念 |
| `NothrowHashable` | 不抛异常的可哈希类型概念 |

### 3.2 调试支持

* 提供断言机制，用于调试和开发阶段的错误检查
* 提供调试输出功能，用于开发阶段的信息输出
* 支持条件编译的调试特性

### 3.3 生命周期管理

* 提供RAII增强支持，确保资源正确释放
* 提供移动语义辅助工具，优化资源管理
* 提供对象生命周期跟踪工具

### 3.4 通用工具

* 提供各种辅助函数，简化常见操作
* 提供类型转换和检查工具
* 提供编译时计算辅助函数

### 3.5 使用示例

```cpp
// 哈希使用示例
std::string_view str = "hello"; uint64_t hash = Fnv1aString(str);

// 哈希组合示例
uint64_t combined = HashCombine(1, "test", 3.14);

// 范围哈希示例
std::vector<int> vec = {1, 2, 3, 4, 5};
uint64_t rangeHash = HashRange(vec);

// 自定义类型哈希支持
struct Point { int x, y; };

// 通过tag_invoke协议实现哈希
template<>
constexpr uint64_t tag_invoke(hash_t, const Point& p) noexcept {
    return HashCombine(p.x, p.y);
}

// 使用默认哈希器
Point p{10, 20};
DefaultHasher<Point> hasher;
uint64_t pointHash = hasher(p);
```