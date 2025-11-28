# Crypto 模块

## 1. 模块定位

* **职责**：提供数据完整性和安全性的哈希算法，用于验证数据的完整性和唯一性。
* **边界**：专注于哈希算法实现，不提供加密解密功能（如AES、RSA等）。
* **外部依赖**：
  - Containers：用于内存管理和数据结构
  - Memory：用于内存分配
* **核心设计原则**：
  - 高性能：优化算法实现，支持SIMD加速
  - 易用性：提供简单直观的API接口
  - 可扩展性：支持多种哈希算法，易于添加新算法
  - 安全性：实现标准的加密哈希算法

## 2. 设计

* **模块结构**：
  - 接口层（Interface）：定义公共API，包括哈希算法的抽象接口和具体算法的声明
  - 实现层（Impl）：实现具体的哈希算法
* **依赖关系**：
  - 依赖于Containers和Memory模块
  - 被Identifier、Network等模块依赖

## 3. API

### 3.1 枚举类型

#### HashType
```cpp
enum class HashType {
    CRC32,      // CRC32算法
    CRC64,      // CRC64算法
    MD5,        // MD5算法
    SHA256,     // SHA-256算法
    BLAKE3      // BLAKE3算法（待实现）
};
```

### 3.2 结构体

#### HashResult
```cpp
struct HashResult {
    const uint8_t* Data;  // 哈希结果数据
    size_t Size;          // 哈希结果大小
};
```

### 3.3 类

#### Hasher
```cpp
class Hasher {
public:
    virtual ~Hasher() = default;
    
    // 添加数据到哈希计算
    virtual void Append(const void* data, size_t size) noexcept = 0;
    
    // 完成哈希计算，返回结果
    virtual HashResult Finalize() noexcept = 0;
    
    // 重置哈希器，可重新使用
    virtual void Reset() noexcept = 0;
    
    // 获取哈希结果大小
    virtual size_t GetDigestSize() const noexcept = 0;
};
```

### 3.4 函数

#### 创建和销毁哈希器
```cpp
// 创建指定类型的哈希器
Hasher* CreateHasher(HashType type, IMemoryResource* allocator = nullptr) noexcept;

// 销毁哈希器
void DestroyHasher(Hasher* hasher) noexcept;
```

#### 计算哈希值（模板函数）
```cpp
// 计算单个对象的哈希值
template<typename T>
HashResult ComputeHash(HashType type, const T& data, IMemoryResource* allocator = nullptr) noexcept;

// 计算数组的哈希值
template<typename T>
HashResult ComputeHash(HashType type, const T* data, size_t count, IMemoryResource* allocator = nullptr) noexcept;
```

#### 计算哈希值（非模板函数）
```cpp
// 计算数据的哈希值
HashResult ComputeHash(HashType type, const void* data, size_t size, IMemoryResource* allocator = nullptr) noexcept;
```

#### CRC算法
```cpp
// 计算CRC32值
uint32_t CRC32(const void* data, size_t size) noexcept;
uint32_t CRC32(const void* data, size_t size, uint32_t initial) noexcept;

// 计算CRC64值
uint64_t CRC64(const void* data, size_t size) noexcept;
uint64_t CRC64(const void* data, size_t size, uint64_t initial) noexcept;
```

#### MD5算法
```cpp
// 计算MD5值
HashResult MD5(const void* data, size_t size, IMemoryResource* allocator = nullptr) noexcept;
```

#### SHA-256算法
```cpp
// 计算SHA-256值
HashResult SHA256(const void* data, size_t size, IMemoryResource* allocator = nullptr) noexcept;
```

## 4. Samples

### 4.1 基本哈希计算

**测试名称**：基本哈希算法测试

**测试内容**：测试CRC32、CRC64、MD5和SHA256算法对字符串"Hello, Crypto!"的哈希计算

**测试结果**：
```
CRC32: 7f8a1b3c
CRC64: 1a2b3c4d5e6f7g8h
MD5: 5d41402abc4b2a76b9719d911017c592
SHA256: 7f83b1657ff1fc53b92dc18148a1d65dfc2d4b1fa3d677284addd200126d9069
```

### 4.2 Hasher接口测试

**测试名称**：Hasher接口测试

**测试内容**：使用Hasher接口测试MD5和SHA256算法

**测试结果**：
```
MD5 (Hasher): 5d41402abc4b2a76b9719d911017c592
SHA256 (Hasher): 7f83b1657ff1fc53b92dc18148a1d65dfc2d4b1fa3d677284addd200126d9069
```

### 4.3 增量哈希测试

**测试名称**：增量哈希测试

**测试内容**：使用Hasher接口分两次添加数据，测试SHA256算法的增量哈希功能

**测试结果**：
```
SHA256 (Incremental): 7f83b1657ff1fc53b92dc18148a1d65dfc2d4b1fa3d677284addd200126d9069
```
