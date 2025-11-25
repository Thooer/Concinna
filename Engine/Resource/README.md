# ✅ Resource：行业标准、能扩展十年不重写的架构

**“资产 ID 化 + 分级缓存 + 异步加载 + 依赖图 + Streaming 管线 + 引用计数/弱句柄。”**

## **1. Asset Identity（统一资产 ID）**

所有资源统一用类似：

* `AssetID`（稳定 GUID）
* 不依赖路径
* 可被引用、序列化、跨平台

内容路径只是元数据，运行时一律用 ID。

**一句话：资源用 ID，不直接用路径。**

---

## **2. Asset Metadata（元信息）**

每个 AssetID 对应一份 metadata：

* 类型（Texture, Mesh, AnimationClip, Shader…）
* 源路径
* 导入配置（压缩、格式、LOD…）
* 依赖列表（其他 AssetID）

这是构建**资源依赖图**和后续 Streaming 的基础。

---

## **3. Import Pipeline（离线导入）**

所有资源先经过 Import Pipeline，转为运行期格式：

* Texture → GPU 压缩纹理
* Mesh → 压缩顶点/索引 + LOD 生成
* Animation → 压缩曲线 + 统一骨骼
* Shader → 入库 IR（HLSL/SPIRV/MSL/DXIL）

Import Pipeline 输出 **Cooked Asset**，可直接加载。

**和 Unreal/Unity 一样：始终区分“源资源”和“运行时资源”。**

---

## **4. Asset Cache Hierarchy（多级缓存体系）**

运行时资源一般经过三层缓存：

```
Disk Cooked Asset → CPU Cache → GPU Cache
```

* Disk：最终产物
* CPU Cache：解压后的对象（Mesh/Texture Header）
* GPU Cache：真正的 VRAM 物件（Buffer/Texture/Pipeline）

通过引用计数与 LRU 对这三层做生命周期管理。

**一句话：资源要分层，要能在任意层释放再重建。**

---

## **5. Async Loading（异步加载）**

所有加载过程必须异步：

* CPU 线程池（解压 / 解析）
* IO 线程（读取磁盘/网络）
* GPU 异步上传（transfer queue）

加载结果是 `Future<AssetHandle>`，绑定 UI/loading/Streaming。

---

## **6. Dependency Graph（资源依赖图）**

每个 Asset 的 metadata 中保存依赖列表：
Mesh 依赖 Texture
Material 依赖 Shader
Scene 依赖 Mesh/Material/Animation

加载 Asset 时自动加载其依赖（并行/有序）。

这是 Streaming 和 Scene 加载的必需品。

---

## **7. Runtime Handle（资源句柄）**

运行时所有资源访问都用 Handle：

* 强引用：`AssetHandle`
* 弱引用：`WeakAssetHandle`
* 由资源系统自动维护引用计数
* 可感知资源重新加载、卸载

**不要将资源指针存到组件里，永远用 handle。**

---

## **8. Streaming Pipeline（动态资源流式加载）**

当你未来要支持：

* 大世界
* 分区加载
* 大量纹理/地形/网格 LOD
  就需要 Streaming：

流程一般是：

```
需求 → StreamingScheduler → IO + CPU Decode → GPU Upload → Ready
```

你的 Scene（我们上一章决定的）会向 Streaming 发“需求”。

---

# 🧩 Resource 总图（最终形态）

```
Resource System
 ├─ AssetID / Metadata
 ├─ Import Pipeline (Cooker)
 ├─ Cache Hierarchy
 │     ├─ Disk Cooked Assets
 │     ├─ CPU Cache
 │     └─ GPU Cache
 ├─ Async Loader
 ├─ Dependency Graph
 ├─ Streaming Pipeline
 └─ Runtime Handles
```

---

# 🔥 最小可行版本（从这起步不会返工）

* 统一 AssetID
* 基础 Import（到 runtime 数据格式）
* 异步加载（CPU）
* 统一 AssetHandle/WeakHandle
* Disk → CPU → GPU 的两/三级缓存
* Metadata + 依赖列表
* 可选 Streaming（不急）

起步版本能支持 Editor / Game / Streaming 三件事。
