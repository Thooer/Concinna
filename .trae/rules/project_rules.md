# 游戏引擎开发

## **1. 哲学**

强灵活性：迟封装，保留高度定制性
最高控制力：几乎所有内容自研。
可维护性：优先考虑健壮性和后期可扩展性而不是MVP。
第一性原理：如有必要优先创新，否则采纳前沿实现。

## **2. 目录**

```
Concinna/
├── .trae/                      # 项目规则与屏蔽配置
│   └── rules/project_rules.md  # 本文档，核心开发规范
├── Build/                      # 构建输出目录
├── Deprecated/                 # 已废弃代码(禁止修改、使用、构建、借鉴、查看)
├── Docs/                       # 文档
├── Language/                   # 语言层
├── Primitive/                  # `Prm::`：原语层
├── Capability/                 # `Cap::`：能力层
├── System/                     # `Sys::`：子系统层
├── Hub/                        # `Hub::`：引擎版本管理
├── Engine/                     # `Eng::`：引擎层
├── Dev/                        # `Dev::`：编辑器工具层
├── Editor/                     # `Edt::`：编辑器层
├── Plugins/                    # `Plg::`：插件层
├── Projects/                   # `Pjt::`：项目层
├── Web/                        # `Web::`：Web层
├── ThirdParty/                 # `Thd::`：第三方库
│   └── Tracy                   # 性能分析工具
├── Scripts/                    # 脚本层
├── Tools/                      # 自研开发工具
└── 配置文件
```

所有模块遵循统一结构，确保一致性：

```
<ModuleName>/
├── API/                        # (必选)接口，禁止使用伞头文件
├── Impl/                       # (可选)实现目录
│   └── <ImplName>/             #       (可选)接口的实现
├── Sample/                     # (可选)示例与测试
│   └── <SampleName>/           #       (必选)示例/测试
├── SubModule/                  # (可选)嵌套子模块
│   └── <SubmoduleName>/        #       (嵌套)遵循标准结构
├── Docs/                       # (可选)文档
├── README.md                   # 模块设计与使用说明
└── CMakeLists.txt              # 模块构建配置
```

## **3. 构建**
当前平台：Windows平台
- **统一管理**：模块根目录CMakeLists.txt统一管理接口、实现和样例
- **模块化导入**：仅使用 `import <Module>`，禁止 `#include` 和跨模块导入分区，MSVC一定会报错。
- **构建优化**：Windows使用Visual Studio生成器
- **构建函数简述**：
  - `API(<BuildName> <API_DIR> [LINK <deps>])` 定义接口文件集并记录依赖；第一个参数为构建名称，例如 `Primitive.System`
  - `Impl(<BuildName.Backend> <IMPL_ROOT> [LINK <deps>])` 注册具体后端实现；例如 `Primitive.System.Windows`
  - `Build(<Target> TYPE STATIC|SHARED BACKEND <BuildName.Backend> [LINK <deps>])` 创建目标并按“全名后端”选择实现；必须使用完整名
  - `Sample(<Name> <Sample_DIR> [OPTION <flag>] [LINK <deps>])` 定义样例可执行并可通过开关控制是否构建
  - `Collect(<OUT_VAR> <DIR>)` 收集目录下的 C++ 模块源文件（`.ixx/.cppm`）用于复用
- **构建命令**：
  - `cmake -S . -B Build` 生成Visual Studio项目


## **4. 规范**
除了少部分标准库，图形学API，文件编解码，编程语言交互库，深度学习，不依赖任何外部库。
### **4.1 核心原则**
- **移动优先**：默认可移动、不可拷贝，拷贝需显式允许
- **RAII管理**：作用域即生命周期，严禁资源泄漏
- **类型安全**：禁止隐式转换、强类型枚举、统一Result/Status返回值
- **性能友好**：`noexcept`默认、值返回优化、完美转发支持

### **4.2 命名规范**
- **类型/方法**：`PascalCase`，常量`CONSTANT`，局部变量`kCONSTANT`
- **成员变量**：公开`m_`，私有`t_`，接口`I`，模板`T`
- **异步接口**：异步`Async`，线程安全`Safe`，非阻塞`Try`
- **命名空间**：最多一层，如`Prm::`
- **模块前缀**：和命名空间一致，如`Prm.`

### **4.3 模块导入**
- **模块化导入**:仅使用 `import <Module>`，禁止 `#include`
- **标准库隔离**:Primitive以外的模块禁止使用，测试文件除外

### **4.4 模块开发流程**

- **模块定位**
- **依赖关系**
- **确定API**：尽可能一个类对应一个文件
- **编写README.md**
- **编写测试**
- **后端实现**

## **5. 架构**

依赖关系：
FoundationLayer：Language->Primitive->Capability->System
EngineLayer：Engine->Dev->Editor
FoundationLayer->EngineLayer
Hub和Web各自独立

### 5.1 基础设施
#### Language层：编程范式

- Element：元素原语模块
- Meta：元编程原语模块
- Paradigm：范式原语模块
- Semantics：语义原语模块
- Flow：流程原语模块
- Text：文本原语模块
- Reflection：反射原语模块

#### Primitive层：重原语，轻逻辑

- Sync：同步原语模块
- Audio：音频原语模块
- Clipboard：剪贴板原语模块
- Debug：平台调试
- DynamicLibrary：动态库原语模块
- File：文件原语模块
- Input：输入原语模块
- IO：IO原语模块
- Ownership：所有权原语模块
- SIMD：SIMD原语模块
- Socket：套接字原语模块
- System：系统原语模块
- Threading：线程原语模块
- Time：时间原语模块
- WSI：窗口系统集成原语模块
- Window：窗口原语模块

#### Capability层：重逻辑，轻状态

- Algorithms：算法能力模块
- Concurrency：并发能力模块
- Config：配置能力模块
- Containers：容器能力模块
- Crypto：加密能力模块
- Debug：调试能力模块
- Identifiier：标识符能力模块
- IO：IO能力模块
- Job：作业能力模块
- Log：日志能力模块
- Math：数学能力模块
- Memory：内存能力模块
- Network：网络能力模块
- Plugin：插件能力模块
- Profiling：埋点与数据捕获
- Random：随机数能力模块
- Reflection：反射能力模块
- Resource：资源能力模块
- Serialization：序列化能力模块
- Streaming：流处理能力模块

#### System层：重状态，轻业务

- HotReload：热重载子系统
- IR：中间表示子系统
- Plugin：插件子系统
- Profiling：会话管理与服务
- ResourceManager：资源管理子系统
- Scripting：脚本子系统
- Streaming：流处理子系统
- Task：任务子系统
- Test：测试子系统
- VFS：虚拟文件系统子系统

### 5.2 引擎业务

#### Engine层：Runtime

- Animation：动画引擎模块
- Audio：音频引擎模块
- Gameplay：游戏玩法引擎模块
- Messaging：消息引擎模块
- ModuleSystem：模块系统引擎模块
- Physics：物理引擎模块
- Renderer：渲染引擎模块
- Resource：资源引擎模块
- Runtime：运行时引擎模块
- Scene：场景引擎模块

#### Dev层：Offline

- Baker：资源烘焙工具
- Coding：编码工具
- Compiler：编译器工具
- Cooking：资源烹饪工具
- DataBase：数据库工具
- Importer：资源导入工具
- Packing：资源打包工具

#### Editor层：
- Controller：控制器模块
- GUI：图形用户界面模块
- Infrastructure：基础设施模块
- Interaction：交互模块
- Plugins：插件模块
- Sub-Editors：子编辑器模块
- WorkSpace：工作区模块

### 5.3 应用程序业务

#### Hub层：引擎版本管理，负责所有内容的安装、启动、更新、卸载等

#### Web层：负责引擎网站前后端，包括文档、示例、插件等

### 5.4 工作区

#### Plugins：专门存储插件的位置

#### Projects：专门存储项目的位置

#### Assets：专门存储项目的资源位置

### 5.5 项目外部

#### Scripts：脚本层，包含构建脚本和工具脚本
  - CMake：CMake构建脚本
  - MoveSource：资源迁移工具脚本

#### ThirdParty：第三方库
  - Tracy：性能分析工具

#### Docs：文档目录，包含引擎设计文档和知识库
  - 知识库：引擎各模块设计文档
  - README文档规范：文档编写规范
  - 报告：项目报告文档
  - 设计：设计文档

> 本章所有内容都是目录的语义，无逻辑区分，只承担命名空间的作用
> 真正的模块在对应的目录下，扁平化分布。
