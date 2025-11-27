# Paradigm 模块

## 1. 模块定位

* **职责**：提供核心编程范式支持，包括位掩码操作、函数视图和强类型别名，为引擎提供统一的编程范式基础。
* **边界**：不提供复杂的业务逻辑，仅专注于核心编程范式构建块；不依赖外部库，仅使用C++标准库和Element模块。
* **外部依赖**：C++标准库、Element模块。

## 2. 设计

### 2.1 核心设计原则

* **高性能**：所有组件设计优先考虑性能，避免不必要的运行时开销
* **类型安全**：通过强类型设计确保编程范式的类型安全
* **易用性**：提供简洁易用的API，降低编程范式的使用门槛
* **可扩展性**：设计开放的编程范式框架，支持后续扩展

### 2.2 模块结构

* **BitMask**：位掩码操作支持，包括位运算操作符重载、HasAny/HasAll检查、Flags模板类
* **FunctionView**：轻量级函数视图，支持普通函数和noexcept函数，用于非拥有式的函数调用
* **StrongAlias**：强类型别名，用于创建类型安全的别名类型

## 3. API

### 3.1 核心类型

| 类型 | 描述 |
|------|------|
| `FunctionView<R(Args...)>` | 普通函数视图，用于非拥有式的函数调用 |
| `FunctionView<R(Args...) noexcept>` | noexcept函数视图，仅接受不抛异常的可调用对象 |
| `Flags<E>` | 位标志集合，支持迭代和位操作 |
| `StrongAlias<Tag, T>` | 强类型别名，创建类型安全的别名类型 |

### 3.2 位掩码操作

| 函数/操作符 | 描述 |
|-------------|------|
| `Bit(x)` | 创建指定位置的位掩码 |
| `operator|`, `operator&`, `operator^`, `operator~` | 位运算操作符重载 |
| `operator|=`, `operator&=`, `operator^=` | 位运算赋值操作符重载 |
| `HasAny(value, flags)` | 检查是否包含任何指定标志 |
| `HasAll(value, flags)` | 检查是否包含所有指定标志 |
| `Flags<E>::Test(f)` | 检查是否包含指定标志 |
| `Flags<E>::Set(f)` | 设置指定标志 |
| `Flags<E>::Unset(f)` | 清除指定标志 |
| `Flags<E>::Empty()` | 检查是否为空 |

### 3.3 函数视图操作

| 方法 | 描述 |
|------|------|
| `FunctionView::Bind(f)` | 绑定函数对象 |
| `FunctionView::HasTarget()` | 检查是否绑定了目标函数 |
| `FunctionView::Reset()` | 重置函数视图 |
| `FunctionView::operator()` | 调用绑定的函数 |

### 3.4 强类型别名

* 提供类型安全的别名类型，避免隐式转换
* 支持所有基础操作，包括算术运算、比较运算等
* 提供显式转换和隐式转换控制

### 3.5 使用示例

```cpp
// 位掩码使用示例
enum class Options { None = 0, Option1 = 1, Option2 = 2 };
template<> struct EnableBitmaskOperators<Options> : std::true_type {};

Options opts = Options::Option1 | Options::Option2;
if (HasAll(opts, Options::Option1)) {
    // 处理逻辑
}

// 函数视图使用示例
int Add(int a, int b) { return a + b; }
FunctionView<int(int, int)> func(Add);
int result = func(1, 2); // 结果为3

// 强类型别名使用示例
struct MeterTag {};
using Meter = StrongAlias<MeterTag, double>;

struct KilometerTag {};
using Kilometer = StrongAlias<KilometerTag, double>;

Meter m = Meter{1000.0};
Kilometer km = Kilometer{1.0}; // 类型安全，不会与Meter隐式转换
```