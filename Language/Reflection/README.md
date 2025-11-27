# Reflection 模块

## 1. 模块定位

* **职责**：提供编译时反射支持，包括反射结构体、反射枚举、类型ID和类型信息，为引擎提供统一的反射机制。
* **边界**：仅提供编译时反射功能，不支持运行时反射；不依赖外部库，仅使用C++标准库和Element模块。
* **外部依赖**：C++标准库、Element模块。

## 2. 设计

### 2.1 核心设计原则

* **编译时优先**：所有反射信息在编译时生成，减少运行时开销
* **类型安全**：通过强类型设计确保反射操作的类型安全
* **易用性**：提供简洁易用的API，降低反射的使用门槛
* **可扩展性**：设计开放的反射框架，支持后续扩展

### 2.2 模块结构

* **Reflectable**：反射概念定义，包括Reflectable和ReflectableEnum
* **TypeID**：类型唯一标识符生成和管理
* **TypeInfo**：类型信息获取和管理
* **MemberInfo**：成员信息描述，用于反射结构体
* **Describe**：反射类型描述，提供统一的反射接口
* **Traversal**：反射成员遍历支持
* **EnumFuncs**：枚举反射功能支持

## 3. API

### 3.1 核心概念

| 概念 | 描述 |
|------|------|
| `Reflectable` | 反射结构体概念，要求类型提供GetTypeName()和GetMemberCount() |
| `ReflectableEnum` | 反射枚举概念，要求枚举提供DescribeEnum特化 |

### 3.2 核心类型

| 类型 | 描述 |
|------|------|
| `TypeID` | 类型唯一标识符 |
| `TypeInfo<T>` | 类型信息获取 |
| `MemberInfo<T>` | 成员信息描述 |
| `Describe<T>` | 反射类型描述 |
| `EnumMemberInfo<E>` | 枚举成员信息 |

### 3.3 反射操作

| 函数 | 描述 |
|------|------|
| `GetTypeName<T>()` | 获取类型名称 |
| `GetMemberCount<T>()` | 获取成员数量 |
| `GetMemberInfo<T, I>()` | 获取指定索引的成员信息 |
| `GetEnumTypeName<E>()` | 获取枚举类型名称 |
| `GetEnumMemberCount<E>()` | 获取枚举成员数量 |
| `GetEnumMembers<E>()` | 获取枚举成员列表 |
| `ForEachMember<T>(f)` | 遍历类型的所有成员 |
| `GetMemberRef<T, I>(v)` | 获取指定索引的成员引用 |

### 3.4 类型信息

| 函数 | 描述 |
|------|------|
| `GetTypeID<T>()` | 获取类型的唯一标识符 |
| `GetTypeSize<T>()` | 获取类型大小 |
| `GetTypeName<T>()` | 获取类型名称 |

### 3.5 枚举反射

| 函数 | 描述 |
|------|------|
| `GetEnumTypeName<E>()` | 获取枚举类型名称 |
| `GetEnumMemberCount<E>()` | 获取枚举成员数量 |
| `GetEnumMembers<E>()` | 获取枚举成员列表 |
| `FromUnderlying<E>(v)` | 将底层类型转换为枚举类型 |
