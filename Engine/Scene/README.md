# ✅ Scene：行业最成熟、可扩展十年的架构

**“节点层（变换/层级） + 数据层（组件/ECS） + 空间层（BVH/Region/Octree）三层分离。”**

## **1. Node Layer（Scene Graph / Transform Graph）**

职责极单纯：**只做层级与变换**

* SceneNode：Parent/Children
* Local/World Transform
* Dirty 标记 + 层级更新（压缩路径）
* 不存放任何逻辑或资源引用
* 不负责空间查询
  这是现代引擎共识。
  Unreal/Unity/Godot 全部拆出“Transform 节点”。

**一句话：SceneNode 只做 Transform，不做 GameObject。**

---

## **2. Data Layer（ECS 或 Component Set）**

Scene 中的“东西”（Mesh、Light、Camera、Collider）不放在节点上，而是独立的数据组件。

两种实现都行：

* **ECS（Archetype/Chunk）**：最高可扩展性
* **普通组件系统**：容易实现但后期要重构

组件通过 **EntityID** 引用 Node 的 Transform。
Transform 自身不是组件，而是 SceneNode 的固定部分。

**一句话：SceneNode 负责位置，Entity/Component 负责数据。**

---

## **3. Space Layer（BroadPhase for Scene）**

Scene 自身也要加速结构（和物理分离）用于：

* 渲染剔除（Frustum/LOD）
* 光照剔除
* 地形/区域划分
* Streaming（分区加载）

推荐结构：

* 全局：**Region / Sector 分区**
* 每区：**BVH（AABB Tree）或 Octree**
* 可选：**Cluster / Cell**（参考 UE5 World Partition）

变换变化 → 更新 Node → 通知 Space Layer → 更新 BVH。

**一句话：Scene 有自己的 BVH，不依赖物理的 BVH。**

---

# 🧩 Scene 总图（最终形态）

```
Scene
 ├─ Node Layer
 │    └─ SceneNode (Transform Only)
 │
 ├─ Data Layer
 │    └─ ECS/Components (Mesh, Light, Camera, Collider...)
 │
 └─ Space Layer
      ├─ Region / Sector
      └─ BVH / Octree per Region
```

核心机制：

* Node = 变换
* Entity/Component = 数据
* Space = 查询与 Streaming

三者解耦，互相不拖累。

---

# 🎯 最小可行版本（不需要重写的那种）

如果你想慢慢扩展，起步也能做到不返工：

* **SceneNode：父子结构 + 局部/世界矩阵更新**
* **Entity：ID + ComponentBag（轻 ECS）**
* **SceneSpace：简单 BVH 或 Grid**
* **变换变化：Node 脏 → Space 更新 → 渲染/物理可查询**

这套结构可以一直进化到 UE5/Unity 的级别，不会推翻。