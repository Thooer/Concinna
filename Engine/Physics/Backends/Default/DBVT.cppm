module;
module Engine.Physics:DBVT;

import Lang;
import Cap.Containers;
import Cap.Memory;
import :Types;

namespace Engine::Physics {

  struct DBVTNode {
    AABB box{};
    Entity entity{0};
    Int32 parent{-1};
    Int32 left{-1};
    Int32 right{-1};
    bool used{false};
    bool isLeaf() const noexcept { return left == -1 && right == -1; }
  };

  struct DBVTTree {
    Int32 root{-1};
    Cap::IAllocator* alloc{nullptr};
  };

  static DBVTTree& tree() noexcept {
    static DBVTTree t{};
    if (!t.alloc) {
      auto r = Cap::CreateDefaultAllocator();
      t.alloc = r.IsOk() ? r.OkValue() : nullptr;
    }
    return t;
  }

  static Foundation::Containers::Vector<DBVTNode>& nodes() noexcept {
    auto& t = tree();
    static Foundation::Containers::Vector<DBVTNode> v(t.alloc);
    return v;
  }

  static Foundation::Containers::Vector<Int32>& freeList() noexcept {
    auto& t = tree();
    static Foundation::Containers::Vector<Int32> v(t.alloc);
    return v;
  }

  static Foundation::Containers::Vector<Entity>& queryBuf() noexcept {
    auto& t = tree();
    static Foundation::Containers::Vector<Entity> v(t.alloc);
    return v;
  }

  static inline AABB combine(const AABB& a, const AABB& b) noexcept {
    AABB c{};
    c.min = { (a.min.x < b.min.x) ? a.min.x : b.min.x,
              (a.min.y < b.min.y) ? a.min.y : b.min.y,
              (a.min.z < b.min.z) ? a.min.z : b.min.z };
    c.max = { (a.max.x > b.max.x) ? a.max.x : b.max.x,
              (a.max.y > b.max.y) ? a.max.y : b.max.y,
              (a.max.z > b.max.z) ? a.max.z : b.max.z };
    return c;
  }

  static inline bool overlap(const AABB& a, const AABB& b) noexcept {
    if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
    if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
    if (a.max.z < b.min.z || a.min.z > b.max.z) return false;
    return true;
  }

  static inline Float32 area(const AABB& b) noexcept {
    Float32 dx = b.max.x - b.min.x;
    Float32 dy = b.max.y - b.min.y;
    Float32 dz = b.max.z - b.min.z;
    return 2.0f * (dx * dy + dy * dz + dz * dx);
  }

  static Int32 allocNode() noexcept {
    auto& fl = freeList();
    auto& ns = nodes();
    if (fl.size() > 0) {
      auto idx = fl.data()[fl.size() - 1];
      (void)fl.resize(fl.size() - 1);
      ns.data()[static_cast<size_t>(idx)].used = true;
      ns.data()[static_cast<size_t>(idx)].parent = -1;
      ns.data()[static_cast<size_t>(idx)].left = -1;
      ns.data()[static_cast<size_t>(idx)].right = -1;
      ns.data()[static_cast<size_t>(idx)].entity = 0;
      return idx;
    }
    DBVTNode n{};
    (void)ns.push_back(n);
    auto idx = static_cast<Int32>(ns.size() - 1);
    ns.data()[static_cast<size_t>(idx)].used = true;
    return idx;
  }

  static void freeNode(Int32 idx) noexcept {
    auto& ns = nodes();
    auto& fl = freeList();
    ns.data()[static_cast<size_t>(idx)].used = false;
    (void)fl.push_back(idx);
  }

  static void refitUp(Int32 idx) noexcept {
    auto& t = tree();
    auto& ns = nodes();
    while (idx != -1) {
      auto& n = ns.data()[static_cast<size_t>(idx)];
      if (!n.isLeaf()) {
        auto& l = ns.data()[static_cast<size_t>(n.left)];
        auto& r = ns.data()[static_cast<size_t>(n.right)];
        n.box = combine(l.box, r.box);
      }
      idx = n.parent;
    }
  }

  static Int32 pickSibling(Int32 start, const AABB& box) noexcept {
    auto& t = tree();
    auto& ns = nodes();
    Int32 idx = start;
    while (!ns.data()[static_cast<size_t>(idx)].isLeaf()) {
      auto& n = ns.data()[static_cast<size_t>(idx)];
      auto& l = ns.data()[static_cast<size_t>(n.left)];
      auto& r = ns.data()[static_cast<size_t>(n.right)];
      AABB c = combine(n.box, box);
      Float32 cost = area(c);
      Float32 costL = area(combine(l.box, box));
      Float32 costR = area(combine(r.box, box));
      if (cost < costL && cost < costR) break;
      idx = (costL < costR) ? n.left : n.right;
    }
    return idx;
  }

  static Int32 findLeafByEntity(Entity e) noexcept {
    auto& ns = nodes();
    for (USize i = 0; i < ns.size(); ++i) {
      auto& n = ns.data()[static_cast<size_t>(i)];
      if (n.used && n.isLeaf() && n.entity == e) return static_cast<Int32>(i);
    }
    return -1;
  }

  static bool insert(Entity e, const AABB& box) noexcept {
    auto& t = tree();
    auto& ns = nodes();
    Int32 leaf = allocNode();
    ns.data()[static_cast<size_t>(leaf)].box = box;
    ns.data()[static_cast<size_t>(leaf)].entity = e;
    ns.data()[static_cast<size_t>(leaf)].left = -1;
    ns.data()[static_cast<size_t>(leaf)].right = -1;
    ns.data()[static_cast<size_t>(leaf)].parent = -1;
    if (t.root == -1) { t.root = leaf; return true; }
    Int32 sibling = pickSibling(t.root, box);
    Int32 oldParent = ns.data()[static_cast<size_t>(sibling)].parent;
    Int32 parent = allocNode();
    auto& p = ns.data()[static_cast<size_t>(parent)];
    p.left = sibling; p.right = leaf; p.parent = oldParent; p.entity = 0;
    p.box = combine(ns.data()[static_cast<size_t>(sibling)].box, box);
    ns.data()[static_cast<size_t>(sibling)].parent = parent;
    ns.data()[static_cast<size_t>(leaf)].parent = parent;
    if (oldParent == -1) {
      t.root = parent;
    } else {
      auto& op = ns.data()[static_cast<size_t>(oldParent)];
      if (op.left == sibling) op.left = parent; else op.right = parent;
    }
    refitUp(parent);
    return true;
  }

  static bool remove(Entity e) noexcept {
    auto& t = tree();
    auto& ns = nodes();
    Int32 leaf = findLeafByEntity(e);
    if (leaf == -1) return false;
    if (leaf == t.root) { freeNode(leaf); t.root = -1; return true; }
    Int32 parent = ns.data()[static_cast<size_t>(leaf)].parent;
    Int32 grand = ns.data()[static_cast<size_t>(parent)].parent;
    Int32 sibling = (ns.data()[static_cast<size_t>(parent)].left == leaf)
      ? ns.data()[static_cast<size_t>(parent)].right
      : ns.data()[static_cast<size_t>(parent)].left;
    if (grand != -1) {
      auto& g = ns.data()[static_cast<size_t>(grand)];
      if (g.left == parent) g.left = sibling; else g.right = sibling;
      ns.data()[static_cast<size_t>(sibling)].parent = grand;
      refitUp(grand);
    } else {
      t.root = sibling;
      ns.data()[static_cast<size_t>(sibling)].parent = -1;
    }
    freeNode(parent);
    freeNode(leaf);
    return true;
  }

  static bool update(Entity e, const AABB& box) noexcept {
    auto& t = tree();
    auto& ns = nodes();
    Int32 leaf = findLeafByEntity(e);
    if (leaf == -1) return insert(e, box);
    AABB old = ns.data()[static_cast<size_t>(leaf)].box;
    if (overlap(old, box) && (area(box) <= area(combine(old, box)))) {
      ns.data()[static_cast<size_t>(leaf)].box = box;
      refitUp(ns.data()[static_cast<size_t>(leaf)].parent);
      return true;
    }
    (void)remove(e);
    return insert(e, box);
  }

  static BroadPhaseView queryAABB(const AABB& q) noexcept {
    auto& t = tree();
    auto& qb = queryBuf();
    qb.clear();
    if (t.root == -1) return { nullptr, 0 };
    Foundation::Containers::Vector<Int32> stack(t.alloc);
    (void)stack.push_back(t.root);
    while (stack.size() > 0) {
      auto idx = stack.data()[stack.size() - 1];
      (void)stack.resize(stack.size() - 1);
      auto& n = nodes().data()[static_cast<size_t>(idx)];
      if (!overlap(n.box, q)) continue;
      if (n.isLeaf()) { (void)qb.push_back(n.entity); }
      else { (void)stack.push_back(n.left); (void)stack.push_back(n.right); }
    }
    BroadPhaseView v{}; v.entities = qb.data(); v.count = qb.size(); return v;
  }

  static inline bool rayAabb(const AABB& box, const Vec3& o, const Vec3& d, Float32 maxDist, Float32& outT) noexcept {
    Float32 tmin = 0.0f;
    Float32 tmax = maxDist;
    auto checkAxis = [&](Float32 origin, Float32 dir, Float32 minB, Float32 maxB) noexcept {
      if (dir == 0.0f) { if (origin < minB || origin > maxB) { tmin = 1.0f; tmax = 0.0f; } return; }
      Float32 inv = 1.0f / dir;
      Float32 t1 = (minB - origin) * inv;
      Float32 t2 = (maxB - origin) * inv;
      if (t1 > t2) { Float32 tmp = t1; t1 = t2; t2 = tmp; }
      if (t1 > tmin) tmin = t1;
      if (t2 < tmax) tmax = t2;
    };
    checkAxis(o.x, d.x, box.min.x, box.max.x);
    checkAxis(o.y, d.y, box.min.y, box.max.y);
    checkAxis(o.z, d.z, box.min.z, box.max.z);
    if (tmax >= tmin && tmin <= maxDist && tmax >= 0.0f) { outT = tmin < 0.0f ? tmax : tmin; return true; }
    return false;
  }

  static Hit raycast(const RayQuery& rq) noexcept {
    Hit best{};
    Float32 len2 = rq.dir.x * rq.dir.x + rq.dir.y * rq.dir.y + rq.dir.z * rq.dir.z;
    if (len2 <= 0) return best;
    Float32 maxDist = static_cast<Float32>(Sqrt(static_cast<double>(len2)));
    Float32 invLen = 1.0f / maxDist;
    Vec3 d{ rq.dir.x * invLen, rq.dir.y * invLen, rq.dir.z * invLen };
    auto& t = tree();
    if (t.root == -1) return best;
    Foundation::Containers::Vector<Int32> stack(t.alloc);
    (void)stack.push_back(t.root);
    Float32 bestT = maxDist;
    while (stack.size() > 0) {
      auto idx = stack.data()[stack.size() - 1];
      (void)stack.resize(stack.size() - 1);
      auto& n = nodes().data()[static_cast<size_t>(idx)];
      Float32 nodeT = 0.0f;
      if (!rayAabb(n.box, rq.origin, d, bestT, nodeT)) continue;
      if (n.isLeaf()) {
        best.hit = true; best.entity = n.entity;
        bestT = nodeT;
        best.pos = { rq.origin.x + d.x * bestT, rq.origin.y + d.y * bestT, rq.origin.z + d.z * bestT };
      } else {
        (void)stack.push_back(n.left); (void)stack.push_back(n.right);
      }
    }
    return best;
  }

  BroadPhaseView DBVT_QueryAABB(const AABB& q) noexcept { return queryAABB(q); }
  bool DBVT_Insert(Entity e, const AABB& a) noexcept { return insert(e, a); }
  bool DBVT_Remove(Entity e) noexcept { return remove(e); }
  bool DBVT_Update(Entity e, const AABB& a) noexcept { return update(e, a); }
  Hit DBVT_Raycast(const RayQuery& rq) noexcept { return raycast(rq); }
}