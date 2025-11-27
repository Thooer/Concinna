# Text 模块

## 1. 模块定位

* **职责**：提供文本处理功能，包括字符串视图、静态字符串和格式化功能，为引擎提供统一的文本处理基础。
* **边界**：不提供复杂的文本算法或正则表达式，仅专注于基础文本构建块；不依赖外部库，仅使用C++标准库和Element模块。
* **外部依赖**：C++标准库、Element模块。

## 2. 设计

### 2.1 核心设计原则

* **高性能**：所有组件设计优先考虑性能，避免不必要的内存分配和拷贝
* **类型安全**：通过强类型设计确保文本操作的类型安全
* **易用性**：提供简洁易用的API，降低文本处理的使用门槛
* **可扩展性**：设计开放的文本框架，支持后续扩展

### 2.2 模块结构

* **StringView**：字符串视图，提供对字符串的非拥有式访问
* **StaticString**：静态字符串，用于编译时已知长度的字符串
* **FormatArg**：格式化参数，用于类型安全的格式化
* **FormatResult**：格式化结果，包含格式化状态和写入字符数
* **Format**：格式化功能，支持类型安全的字符串格式化

## 3. API

### 3.1 核心类型

| 类型 | 描述 |
|------|------|
| `StringView` | 字符串视图，提供对字符串的非拥有式访问 |
| `StaticString<N>` | 静态字符串，用于编译时已知长度的字符串 |
| `FormatArg` | 格式化参数，用于类型安全的格式化 |
| `FormatResult` | 格式化结果，包含格式化状态和写入字符数 |
| `FormatPack<N>` | 格式化参数包，用于存储多个格式化参数 |

### 3.2 字符串视图操作

* 提供对字符串的非拥有式访问，避免不必要的内存分配和拷贝
* 支持字符串比较、查找、子串等常见操作
* 支持与C风格字符串和std::string的互操作

### 3.3 静态字符串

* 用于编译时已知长度的字符串，提供更高的性能
* 支持编译时字符串操作和计算
* 支持与StringView的互操作

### 3.4 格式化功能

| 函数/类型 | 描述 |
|-----------|------|
| `MakeArg(value)` | 创建格式化参数 |
| `CaptureArgs(values...)` | 捕获多个格式化参数 |
| `FormatTo(out, fmt, args)` | 格式化到指定缓冲区 |
| `FormatStatus` | 格式化状态枚举 |

### 3.5 格式化状态

| 状态 | 描述 |
|------|------|
| `Ok` | 格式化成功 |
| `Truncated` | 输出被截断 |
| `InvalidSpec` | 无效的格式化规范 |

### 3.6 使用示例

```cpp
// 字符串视图使用示例
StringView sv = "hello world";
StringView substr = sv.Substring(0, 5); // "hello"

// 静态字符串使用示例
StaticString<5> staticStr = "hello";

// 格式化使用示例
Char8 buffer[64];
Span<Char8> out(buffer);
StringView fmt = "Hello, {}! The answer is {}.";
auto args = CaptureArgs("world", 42);
FormatResult result = FormatTo(out, fmt, args.AsSpan());
if (result.status == FormatStatus::Ok) {
    // 格式化成功，使用buffer中的内容
}
```