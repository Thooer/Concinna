# ✅ Animation：现代引擎一致采用的最终级架构

**“统一骨骼 + 动画 Graph + Blend 树 + 曲线/约束驱动 + Retarget 层 + CPU/GPU 解算可切换。”**

## **1. Unified Skeleton（统一骨骼体系）**

所有动画资产共享：

* 统一骨骼结构（Human/Humanoid/Creature）
* 每个动画 clip 绑定同一套骨骼ID
* 运行时的 SkeletonInstance 只是 Pose 缓存

**一句话：骨骼是“类型”，动画是“数据”，实例只有 Pose。**

---

## **2. Animation Clip（压缩曲线 Clip）**

Clip 是最基础的资产，包含：

* 每根骨骼的曲线（T/R/S）
* 时间轴（范围、速率）
* 压缩（曲线量化、Key reduction）

保证：

* 快速 sample
* 可用于 Blend
* 适合 Stream（大世界）

---

## **3. Animation Graph（核心运行层）**

这是现代引擎动画系统的核心（UE AnimGraph、Unity Playables）。

Graph 内部一般由节点构成：

* State Machine（Idle/Run/Jump）
* Blend Node（BlendSpace 1D/2D）
* Layer Node（上半身/下半身分层）
* Additive Node
* IK Node
* Procedural Node（LookAt/FootPlacement）
* Curve Node（驱动参数）

数据流是：
**Graph → Blended Pose → (IK/PostProcess) → Final Pose**

**一句话：所有动画逻辑在 Graph，不写在行为代码里。**

---

## **4. Blend System（核心技术点）**

Blend 支持：

* Linear Blend（LERP）
* Additive
* BlendSpace 1D
* BlendSpace 2D
* Per-bone blending（Mask）

这是动画自然度的关键。

---

## **5. Retarget Layer（重定向层）**

现代动画系统必须支持：

* 不同角色骨骼不同
* 动画自动匹配骨骼
* 分关节规则映射
* 基于 Human Description 的自动 retarget（类似 Unity）

**一句话：动画不用绑死角色骨骼，通过 Retarget 自动兼容。**

---

## **6. Constraints（约束系统）**

用于动态后处理：

* Aim
* IK（FABRIK / CCD）
* CCD chain
* LookAt
* Pole vector
* Foot placement（地面对齐）

最终导向更自然、更物理可信的 Pose。

---

## **7. Runtime Pose Buffer（Pose 缓存层）**

Graph 输出的 Pose 写进 PoseBuffer：

* 当前 Pose
* 累积（Additive） Pose
* GPU Skinning 输入
* IK/PostProcess 修改的最终数据

**Scene/渲染系统只读最终 Pose，不介入计算。**

---

## **8. CPU/GPU Evaluation（解算后端可切换）**

早期：

* CPU 计算 Pose / Blend / IK

后来：

* GPU 动画（Compute）
* GPU Blend（Texture driven）
* GPU Skinning（Vertex Shader / Compute Skinned Buffer）

如果结构正确，从 CPU 迁移到 GPU 不需要重写上层。

---

# 🧩 Animation 总图（最终形态）

```
Animation System
 ├─ Skeleton (Definition)
 ├─ Animation Clips (Compressed Curves)
 ├─ Animation Graph
 │     ├─ State Machines
 │     ├─ Blend Nodes
 │     ├─ Layer / Mask
 │     ├─ Additive
 │     ├─ Procedural Nodes
 │     └─ IK / Constraints
 ├─ Retarget Layer
 ├─ Pose Buffer (Final Pose)
 └─ CPU/GPU Evaluation
```

---

# 🔥 最小可行版本（从这起步未来不用重写）

* 统一 Skeleton + SkeletonInstance
* Clip（曲线 + 时间）
* 基础 Blend（LERP + Additive）
* 基础 Graph（StateMachine + Blend）
* 最终 Pose 输出给渲染系统
* CPU 惯性/简单 IK

未来可以自然演进到：
BlendSpace → Layer → Retarget → GPU 动画 → Constraints。


---
