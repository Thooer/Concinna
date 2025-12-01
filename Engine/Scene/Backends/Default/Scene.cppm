module;
module Engine.Scene;

import Lang;
import Cap.Memory;

namespace Engine::Scene {
    SceneWorld::SceneWorld(Cap::IAllocator* alloc) noexcept
        : m_alloc(alloc), m_entities(nullptr), m_positions(nullptr), m_meshes(nullptr), m_materials(nullptr), m_flags(nullptr), m_parents(nullptr), m_count(0), m_cap(0) {}

    Entity SceneWorld::CreateEntity() noexcept {
        if (m_count == m_cap) {
            USize newCap = (m_cap == 0) ? static_cast<USize>(8) : (m_cap * static_cast<USize>(2));
            auto er = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(Entity)), static_cast<USize>(alignof(Entity)));
            if (!er.IsOk()) return static_cast<Entity>(m_count);
            Entity* newE = reinterpret_cast<Entity*>(er.OkValue());
            for (USize i=0;i<m_count;++i) newE[static_cast<size_t>(i)] = m_entities[static_cast<size_t>(i)];
            if (m_entities) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(m_entities), m_cap * static_cast<USize>(sizeof(Entity)), static_cast<USize>(alignof(Entity)));
            m_entities = newE;
            auto pr = m_alloc->Allocate((newCap*3) * static_cast<USize>(sizeof(float)), static_cast<USize>(alignof(float)));
            if (!pr.IsOk()) return static_cast<Entity>(m_count);
            float* newP = reinterpret_cast<float*>(pr.OkValue());
            for (USize i=0;i<m_count*3;++i) newP[static_cast<size_t>(i)] = m_positions ? m_positions[static_cast<size_t>(i)] : 0.0f;
            if (m_positions) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(m_positions), (m_cap*3) * static_cast<USize>(sizeof(float)), static_cast<USize>(alignof(float)));
            m_positions = newP;
            auto mr = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(UInt32)), static_cast<USize>(alignof(UInt32)));
            if (!mr.IsOk()) return static_cast<Entity>(m_count);
            UInt32* newM = reinterpret_cast<UInt32*>(mr.OkValue());
            for (USize i=0;i<m_count;++i) newM[static_cast<size_t>(i)] = m_meshes ? m_meshes[static_cast<size_t>(i)] : static_cast<UInt32>(0);
            if (m_meshes) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(m_meshes), m_cap * static_cast<USize>(sizeof(UInt32)), static_cast<USize>(alignof(UInt32)));
            m_meshes = newM;
            auto ar = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(UInt32)), static_cast<USize>(alignof(UInt32)));
            if (!ar.IsOk()) return static_cast<Entity>(m_count);
            UInt32* newA = reinterpret_cast<UInt32*>(ar.OkValue());
            for (USize i=0;i<m_count;++i) newA[static_cast<size_t>(i)] = m_materials ? m_materials[static_cast<size_t>(i)] : static_cast<UInt32>(0);
            if (m_materials) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(m_materials), m_cap * static_cast<USize>(sizeof(UInt32)), static_cast<USize>(alignof(UInt32)));
            m_materials = newA;
            auto fr = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(UInt8)), static_cast<USize>(alignof(UInt8)));
            if (!fr.IsOk()) return static_cast<Entity>(m_count);
            UInt8* newF = reinterpret_cast<UInt8*>(fr.OkValue());
            for (USize i=0;i<m_count;++i) newF[static_cast<size_t>(i)] = m_flags ? m_flags[static_cast<size_t>(i)] : static_cast<UInt8>(0);
            if (m_flags) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(m_flags), m_cap * static_cast<USize>(sizeof(UInt8)), static_cast<USize>(alignof(UInt8)));
            m_flags = newF;
            auto prt = m_alloc->Allocate(newCap * static_cast<USize>(sizeof(Entity)), static_cast<USize>(alignof(Entity)));
            if (!prt.IsOk()) return static_cast<Entity>(m_count);
            Entity* newParents = reinterpret_cast<Entity*>(prt.OkValue());
            for (USize i=0;i<m_count;++i) newParents[static_cast<size_t>(i)] = m_parents ? m_parents[static_cast<size_t>(i)] : static_cast<Entity>(UInt32(-1));
            if (m_parents) (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(m_parents), m_cap * static_cast<USize>(sizeof(Entity)), static_cast<USize>(alignof(Entity)));
            m_parents = newParents;
            m_cap = newCap;
        }
        Entity id = static_cast<Entity>(m_count);
        m_entities[static_cast<size_t>(m_count)] = id;
        USize idx = m_count * static_cast<USize>(3);
        m_positions[static_cast<size_t>(idx+0)] = 0.0f;
        m_positions[static_cast<size_t>(idx+1)] = 0.0f;
        m_positions[static_cast<size_t>(idx+2)] = 0.0f;
        m_meshes[static_cast<size_t>(m_count)] = static_cast<UInt32>(0);
        m_materials[static_cast<size_t>(m_count)] = static_cast<UInt32>(0);
        m_flags[static_cast<size_t>(m_count)] = static_cast<UInt8>(0);
        m_parents[static_cast<size_t>(m_count)] = static_cast<Entity>(UInt32(-1));
        m_count += static_cast<USize>(1);
        return id;
    }

    bool SceneWorld::DestroyEntity(Entity e) noexcept {
        if (static_cast<USize>(e) >= m_count) return false;
        m_entities[static_cast<size_t>(e)] = static_cast<Entity>(UInt32(-1));
        m_flags[static_cast<size_t>(e)] = static_cast<UInt8>(0);
        return true;
    }

    bool SceneWorld::SetTransform(Entity e, const Transform& t) noexcept {
        if (static_cast<USize>(e) >= m_count) return false;
        USize idx = static_cast<USize>(e) * static_cast<USize>(3);
        float* p = m_positions;
        p[static_cast<size_t>(idx + 0)] = t.x;
        p[static_cast<size_t>(idx + 1)] = t.y;
        p[static_cast<size_t>(idx + 2)] = t.z;
        return true;
    }

    StatusResult<Transform> SceneWorld::GetTransform(Entity e) const noexcept {
        if (static_cast<USize>(e) >= m_count) return StatusResult<Transform>::Err(Err(StatusDomain::Memory(), StatusCode::NotFound));
        if (m_entities[static_cast<size_t>(e)] == static_cast<Entity>(UInt32(-1))) return StatusResult<Transform>::Err(Err(StatusDomain::Memory(), StatusCode::NotFound));
        USize idx = static_cast<USize>(e) * static_cast<USize>(3);
        Transform t{ m_positions[static_cast<size_t>(idx + 0)], m_positions[static_cast<size_t>(idx + 1)], m_positions[static_cast<size_t>(idx + 2)] };
        return StatusResult<Transform>::Ok(t);
    }

    bool SceneWorld::SetParent(Entity child, Entity parent) noexcept {
        if (static_cast<USize>(child) >= m_count) return false;
        if (static_cast<USize>(parent) >= m_count) return false;
        if (child == parent) return false;
        m_parents[static_cast<size_t>(child)] = parent;
        return true;
    }

    bool SceneWorld::BindMesh(Entity e, UInt32 h) noexcept {
        if (static_cast<USize>(e) >= m_count) return false;
        m_meshes[static_cast<size_t>(e)] = h; return true;
    }

    bool SceneWorld::BindMaterial(Entity e, UInt32 h) noexcept {
        if (static_cast<USize>(e) >= m_count) return false;
        m_materials[static_cast<size_t>(e)] = h; return true;
    }

    SceneView SceneWorld::GetView() const noexcept { return SceneView{ m_entities, m_positions, m_meshes, m_materials, m_count }; }


    USize SceneWorld::Count() const noexcept { return m_count; }

    bool SceneWorld::AddTransformComponent(Entity e) noexcept {
        if (static_cast<USize>(e) >= m_count) return false;
        m_flags[static_cast<size_t>(e)] |= static_cast<UInt8>(1u);
        return true;
    }
    bool SceneWorld::RemoveTransformComponent(Entity e) noexcept {
        if (static_cast<USize>(e) >= m_count) return false;
        m_flags[static_cast<size_t>(e)] &= static_cast<UInt8>(~static_cast<UInt8>(1u));
        return true;
    }
    bool SceneWorld::HasTransform(Entity e) const noexcept {
        if (static_cast<USize>(e) >= m_count) return false;
        return (m_flags[static_cast<size_t>(e)] & static_cast<UInt8>(1u)) != static_cast<UInt8>(0);
    }
}