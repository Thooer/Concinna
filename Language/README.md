# Language 层架构 README

## 1. 定位

- 职责：提供与平台无关的基础语义与抽象，定义类型系统、概念/约束、范式原语、流程控制与错误处理、文本与格式化等，为全工程的“语言基座”。
- 边界：不直接进行系统调用、不管理线程/IO/窗口、不依赖外部第三方库；不承担平台适配或业务逻辑聚合。
- 核心设计原则：
  - 强类型安全与明确约束（Concepts/CPO/TagInvoke），禁止隐式转换
  - 移动优先、RAII 管理、`noexcept` 友好与值返回优化
  - 模块化导入，仅使用 `import`，严禁跨模块伞头耦合
  - 面向扩展的分区组织（每个模块用 `:Types`/`:Ops`/领域分区）

## 2. 设计

- 模块结构：
  - Element：通用类型与原语（`Types`、`Span`、`Optional`、`Traits`、`Concepts`），为其余模块提供基础构件
  - Meta：元编程原语（`TypeList`、`TypeSet`、`TypeTag`）、CPO/TagInvoke 机制与适配器
  - Paradigm：范式原语（`StrongAlias`、`FunctionView`、`BitMask`），以类型安全的方式封装通用模式
  - Semantics：语义与约定（`Debug`、`Hash`、`Lifetime`、`Utility`），提供断言/哈希等基础设施
  - Flow：流程与错误（`Status`、`Expect`、`StatusDomain/Code`、`Functional`），统一返回值与组合器
  - Text：字符串视图与格式化（`StringView`、`Format` 系列），支持轻量文本处理

- 模块之间依赖关系：
  - Element 为大多数模块的依赖起点
  - Meta 依赖 Element，Paradigm 依赖 Element/Meta
  - Semantics 依赖 Element（断言/哈希等基础设施）
  - Flow 依赖 Element（类型与约束），内部自洽的错误域模型
  - Text 依赖 Element 与 Flow（格式化返回/错误）

- 依赖顺序建议：Element → Meta → Paradigm/Flow/Semantics → Text
