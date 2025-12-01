### 1. 核心原则

1.  **无状态 (Stateless)**：现代 API 的 Command Buffer 是无状态的（或者说状态是显式绑定的）。不要在 RHI 层缓存状态（如“当前绑定的纹理”），直接透传给底层。
2.  **句柄化 (Handle-based)**：不要返回 `ID3D12Resource*` 或 `VkImage` 指针。使用 `Lang.Paradigm` 中的 `StrongAlias` 定义强类型句柄（如 `TextureHandle`, `BufferHandle`）。
3.  **显式同步 (Explicit Sync)**：不要试图自动处理 Barrier。RHI 层应暴露 `ResourceBarrier` 接口，由上层（RenderGraph）决定何时插入。
4.  **预编译对象 (PSO)**：Pipeline State Object 是核心。不要在 Draw Call 时动态拼凑状态。

---

### 2. 模块结构 (`Sys.RHI`)

```text
Sys.RHI/
├── API/
│   ├── Types.ixx          # 格式枚举、句柄定义
│   ├── Device.ixx         # 设备创建、队列管理
│   ├── Resource.ixx       # Buffer/Texture 描述符与创建
│   ├── Pipeline.ixx       # PSO、RootSignature/Layout
│   ├── CommandList.ixx    # 核心：命令录制接口
│   ├── Sync.ixx           # Fence, Semaphore, Barrier
│   └── SwapChain.ixx      # 与 Prm.WSI 对接
├── Impl/
│   ├── Vulkan/            # Vulkan 后端
│   └── DX12/              # DX12 后端
```

---

### 3. 关键组件设计

#### 3.1 类型与句柄 (`Types.ixx`)

利用你现有的 `Lang.Paradigm`：

```cpp
export module Sys.RHI:Types;
import Lang.Paradigm;
import Lang.Element;

export namespace Sys::RHI {
    // 强类型句柄，避免 void* 满天飞，且零成本
    struct BufferTag; using BufferHandle = Prm::StrongAlias<void*, BufferTag>;
    struct TextureTag; using TextureHandle = Prm::StrongAlias<void*, TextureTag>;
    struct PipelineTag; using PipelineHandle = Prm::StrongAlias<void*, PipelineTag>;
    
    enum class Format : UInt32 { R8G8B8A8_UNORM, D32_FLOAT, ... };
    enum class ResourceUsage : UInt8 { Default, Upload, Readback };
}
```

#### 3.2 资源管理 (`Resource.ixx`)

现代 API 的内存分配与资源创建是分离的。但在 RHI 层，为了易用性，通常结合在一起（或者你可以暴露 `Heap` 概念）。

**关键点**：必须区分 **Transient (瞬态)** 和 **Persistent (持久)** 资源。

```cpp
export struct BufferDesc {
    USize size;
    ResourceUsage usage;
    bool isUAV; // 是否允许无序访问写入
};

// 显式内存控制：Concinna 风格
// 这里的实现内部会调用 VMA (Vulkan Memory Allocator) 或 D3D12MA
export Expect<BufferHandle> CreateBuffer(DeviceHandle dev, const BufferDesc& desc);
```

#### 3.3 描述符/绑定模型 (The Binding Model)

这是最难统一的部分。
*   **Vulkan**: Descriptor Sets (Set -> Binding)
*   **DX12**: Root Signature (Root Parameter -> Descriptor Table)

**Concinna 建议方案：基于“绑定组 (BindGroup)”的抽象**。
这类似于 WebGPU 的设计，它是对 Vulkan 和 DX12 的最大公约数。

1.  **RootSignature / PipelineLayout**：定义“插槽”的布局。
2.  **BindGroup / DescriptorSet**：实际填充这些插槽的资源集合。

```cpp
// 定义布局
struct BindingLayoutElement {
    UInt32 binding;
    ResourceType type; // UniformBuffer, Texture, Sampler
    ShaderStage visibility;
};

// 创建一个绑定组（相当于分配一个 Descriptor Set）
export Expect<BindGroupHandle> CreateBindGroup(DeviceHandle dev, const BindGroupDesc& desc);
```

#### 3.4 命令列表 (`CommandList.ixx`)

这是每一帧调用的核心。它必须是**线程安全**的（指不同线程可以录制不同的 CommandList）。

```cpp
export module Sys.RHI:CommandList;
import :Types;

export namespace Sys::RHI {
    struct CommandList {
        // 1. 设置状态
        void SetPipeline(PipelineHandle pso);
        void SetViewport(const Viewport& vp);
        void SetScissor(const Rect& rect);

        // 2. 绑定资源 (Root Signature / Descriptor Sets)
        // setIndex 对应 Vulkan 的 set=N 或 DX12 的 RootParameter Index
        void SetBindGroup(UInt32 setIndex, BindGroupHandle group);
        
        // 3. 绑定顶点/索引缓冲
        void SetVertexBuffer(UInt32 slot, BufferHandle buf);
        void SetIndexBuffer(BufferHandle buf);

        // 4. 绘制
        void Draw(UInt32 vertexCount, UInt32 instanceCount, ...);
        void DrawIndexed(UInt32 indexCount, ...);

        // 5. 同步 (最关键！)
        // 显式插入屏障，转换资源状态 (e.g., RenderTarget -> ShaderResource)
        void ResourceBarrier(Span<const Barrier> barriers);
        
        // 6. 结束
        Status Close();
    };
}
```

---

### 4. 实现策略 (Implementation Strategy)

#### 4.1 内存分配器
不要自己手写 GPU 内存分配器（Buddy Allocator 等），这是个深坑。
*   **推荐**：直接集成 **VulkanMemoryAllocator (VMA)** 和 **D3D12MemoryAllocator (D3D12MA)** 到 `ThirdParty`。
*   **封装**：在 `Sys.RHI` 内部封装它们，对外只暴露 `BufferHandle`。

#### 4.2 交换链 (Swapchain)
利用你已经写好的 `Prm.WSI`。
*   `Sys.RHI` 初始化时，调用 `Prm.WSI::CreateVulkanSurface` 获取 Surface。
*   RHI 负责创建 Swapchain Images。

#### 4.3 统一着色器 (Shader Compilation)
不要写两套 Shader (HLSL/GLSL)。
*   **方案**：统一使用 **HLSL**。
*   **工具**：使用 **DXC (DirectX Shader Compiler)**。
    *   DX12: HLSL -> DXIL
    *   Vulkan: HLSL -> SPIR-V (DXC 原生支持)
*   **位置**：这应该放在 `Dev.AssetCompiler` 模块中，离线编译成二进制，运行时 `Sys.RHI` 直接加载二进制。

---

### 5. 针对 Concinna 的具体建议

鉴于你已经有了 `System.IR` (IRSystem)，你有一个巨大的机会：

**利用 IRSystem 生成 Shader！**

1.  **Shader Graph / Compute Kernel**：用户在编辑器或 C++ DSL 中构建 `MorphismIR`。
2.  **Lowering**：编写一个 Pass，将 `MorphismIR` 转换为 SPIR-V (Vulkan) 或 DXIL (DX12) 的二进制格式，或者生成 HLSL 源代码再调用 DXC。
3.  **优势**：这将使 Concinna 拥有类似 Unity Shader Graph 或 CUDA 的能力，且完全集成在你的 IR 架构中。

### 6. 开发路线图

1.  **Hello Triangle (Vulkan)**：
    *   先只做 Vulkan 后端（跨平台能力强，且最啰嗦，封装了 Vulkan 之后 DX12 很容易适配）。
    *   实现 `Device`, `Queue`, `SwapChain`。
    *   硬编码一个 Shader。
    *   实现最简单的 `CommandList::Begin/End/Draw`。

2.  **资源系统**：
    *   引入 VMA。
    *   实现 `CreateBuffer` (Vertex/Index)。
    *   实现 Staging Buffer (CPU -> GPU 数据上传)。

3.  **描述符系统**：
    *   这是最难的。实现 `BindGroup` 抽象。
    *   在 Vulkan 后端管理 `VkDescriptorPool`。

4.  **多线程录制**：
    *   利用你的 `Sys.Job` 系统。
    *   每一帧：`JobSystem` 分发 N 个任务 -> 每个任务获取一个 `RHI::CommandList` -> 录制 -> 主线程 `QueueSubmit`。

### 7. 示例代码片段 (CommandList 抽象)

```cpp
// Sys.RHI.API.CommandList.ixx
export module Sys.RHI:CommandList;
import Lang;
import :Types;

export namespace Sys::RHI {
    struct ICommandListImpl; // 后端实现接口

    export class CommandList {
        ICommandListImpl* m_impl;
    public:
        // 显式控制，无虚函数开销（如果使用 Pimpl + 模板 或者 纯虚函数取决于你对虚调用的容忍度）
        // 鉴于 RHI 调用频率极高，建议使用 编译期多态 或 函数指针表
        
        void BeginRenderPass(const RenderPassDesc& desc) noexcept {
            m_impl->BeginRenderPass(desc);
        }
        
        void Draw(UInt32 vertexCount) noexcept {
            m_impl->Draw(vertexCount);
        }
        
        // ...
    };
}
```





### 4.1 **CommandList 的动态分派**

虚函数开销。建议：

**最佳策略：函数指针表 (C-style vtable)**

* 无虚表开销
* 易做 Hot Reload
* 多后端切换成本低

### 4.2 **BindGroup 创建频率**

BindGroup 不应在帧中频繁创建
→ 必须支持 **BindGroupCache** 或 **DescriptorPool Reuse**。

否则 Vulkan 会炸 CPU。

### 4.3 **Barrier 插入**

暴露显式 Barrier 是正确的，但记得确保：

* 支持 **batch barrier**（你已经设计了 Span）
* RenderGraph 要能聚合并排序 barrier
* RHI 不能隐式 reorder（保持透明性）

### 4.4 **PSO 管理**

要避免 PSO runtime 拼装
→ 你已经定死 PSO 生成路径，这很好。

要再补一点：

* PSO 创建必须可缓存（哈希 Key）
* Shader Variant 组合必须在 AssetCompiler 完成

---

## 5. 可能的增强点（不展开）

只点最重要的三项：

1. **独立 Sampler Cache**（DX12/VK 都昂贵）
2. **Transient Resource System**（RenderGraph 必备）
3. **CommandAllocator 池**（避免频繁 reset）

---