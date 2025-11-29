module;
module Engine.Scene;

import Lang;
import Foundation.Memory;

namespace Engine::Scene {
    SceneWorld::SceneWorld(::Foundation::Memory::IAllocator* alloc) noexcept
        : m_alloc(alloc), m_entities(nullptr), m_positions(nullptr), m_meshes(nullptr), m_materials(nullptr), m_flags(nullptr), m_parents(nullptr), m_count(0), m_cap(0) {}

    Entity SceneWorld::CreateEntity() noexcept {
        if (m_count == m_cap) {
            Language::USize newCap = (m_cap == 0) ? static_cast<Language::USize>(8) : (m_cap * static_cast<Language::USize>(2));
            auto er = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(Entity)), static_cast<Language::USize>(alignof(Entity)));
            if (!er.IsOk()) return static_cast<Entity>(m_count);
            Entity* newE = reinterpret_cast<Entity*>(er.OkValue());
            for (Language::USize i=0;i<m_count;++i) newE[static_cast<size_t>(i)] = m_entities[static_cast<size_t>(i)];
            if (m_entities) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_entities), m_cap * static_cast<Language::USize>(sizeof(Entity)), static_cast<Language::USize>(alignof(Entity)));
            m_entities = newE;
            auto pr = m_alloc->Allocate((newCap*3) * static_cast<Language::USize>(sizeof(float)), static_cast<Language::USize>(alignof(float)));
            if (!pr.IsOk()) return static_cast<Entity>(m_count);
            float* newP = reinterpret_cast<float*>(pr.OkValue());
            for (Language::USize i=0;i<m_count*3;++i) newP[static_cast<size_t>(i)] = m_positions ? m_positions[static_cast<size_t>(i)] : 0.0f;
            if (m_positions) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_positions), (m_cap*3) * static_cast<Language::USize>(sizeof(float)), static_cast<Language::USize>(alignof(float)));
            m_positions = newP;
            auto mr = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(Language::UInt32)), static_cast<Language::USize>(alignof(Language::UInt32)));
            if (!mr.IsOk()) return static_cast<Entity>(m_count);
            Language::UInt32* newM = reinterpret_cast<Language::UInt32*>(mr.OkValue());
            for (Language::USize i=0;i<m_count;++i) newM[static_cast<size_t>(i)] = m_meshes ? m_meshes[static_cast<size_t>(i)] : static_cast<Language::UInt32>(0);
            if (m_meshes) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_meshes), m_cap * static_cast<Language::USize>(sizeof(Language::UInt32)), static_cast<Language::USize>(alignof(Language::UInt32)));
            m_meshes = newM;
            auto ar = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(Language::UInt32)), static_cast<Language::USize>(alignof(Language::UInt32)));
            if (!ar.IsOk()) return static_cast<Entity>(m_count);
            Language::UInt32* newA = reinterpret_cast<Language::UInt32*>(ar.OkValue());
            for (Language::USize i=0;i<m_count;++i) newA[static_cast<size_t>(i)] = m_materials ? m_materials[static_cast<size_t>(i)] : static_cast<Language::UInt32>(0);
            if (m_materials) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_materials), m_cap * static_cast<Language::USize>(sizeof(Language::UInt32)), static_cast<Language::USize>(alignof(Language::UInt32)));
            m_materials = newA;
            auto fr = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(Language::UInt8)), static_cast<Language::USize>(alignof(Language::UInt8)));
            if (!fr.IsOk()) return static_cast<Entity>(m_count);
            Language::UInt8* newF = reinterpret_cast<Language::UInt8*>(fr.OkValue());
            for (Language::USize i=0;i<m_count;++i) newF[static_cast<size_t>(i)] = m_flags ? m_flags[static_cast<size_t>(i)] : static_cast<Language::UInt8>(0);
            if (m_flags) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_flags), m_cap * static_cast<Language::USize>(sizeof(Language::UInt8)), static_cast<Language::USize>(alignof(Language::UInt8)));
            m_flags = newF;
            auto prt = m_alloc->Allocate(newCap * static_cast<Language::USize>(sizeof(Entity)), static_cast<Language::USize>(alignof(Entity)));
            if (!prt.IsOk()) return static_cast<Entity>(m_count);
            Entity* newParents = reinterpret_cast<Entity*>(prt.OkValue());
            for (Language::USize i=0;i<m_count;++i) newParents[static_cast<size_t>(i)] = m_parents ? m_parents[static_cast<size_t>(i)] : static_cast<Entity>(Language::UInt32(-1));
            if (m_parents) (void)m_alloc->Deallocate(reinterpret_cast<Language::Byte*>(m_parents), m_cap * static_cast<Language::USize>(sizeof(Entity)), static_cast<Language::USize>(alignof(Entity)));
            m_parents = newParents;
            m_cap = newCap;
        }
        Entity id = static_cast<Entity>(m_count);
        m_entities[static_cast<size_t>(m_count)] = id;
        Language::USize idx = m_count * static_cast<Language::USize>(3);
        m_positions[static_cast<size_t>(idx+0)] = 0.0f;
        m_positions[static_cast<size_t>(idx+1)] = 0.0f;
        m_positions[static_cast<size_t>(idx+2)] = 0.0f;
        m_meshes[static_cast<size_t>(m_count)] = static_cast<Language::UInt32>(0);
        m_materials[static_cast<size_t>(m_count)] = static_cast<Language::UInt32>(0);
        m_flags[static_cast<size_t>(m_count)] = static_cast<Language::UInt8>(0);
        m_parents[static_cast<size_t>(m_count)] = static_cast<Entity>(Language::UInt32(-1));
        m_count += static_cast<Language::USize>(1);
        return id;
    }

    bool SceneWorld::DestroyEntity(Entity e) noexcept {
        if (static_cast<Language::USize>(e) >= m_count) return false;
        m_entities[static_cast<size_t>(e)] = static_cast<Entity>(Language::UInt32(-1));
        m_flags[static_cast<size_t>(e)] = static_cast<Language::UInt8>(0);
        return true;
    }

    bool SceneWorld::SetTransform(Entity e, const Transform& t) noexcept {
        if (static_cast<Language::USize>(e) >= m_count) return false;
        Language::USize idx = static_cast<Language::USize>(e) * static_cast<Language::USize>(3);
        float* p = m_positions;
        p[static_cast<size_t>(idx + 0)] = t.x;
        p[static_cast<size_t>(idx + 1)] = t.y;
        p[static_cast<size_t>(idx + 2)] = t.z;
        return true;
    }

    Language::StatusResult<Transform> SceneWorld::GetTransform(Entity e) const noexcept {
        if (static_cast<Language::USize>(e) >= m_count) return Language::StatusResult<Transform>::Err(Language::Err(Language::StatusDomain::Memory(), Language::StatusCode::NotFound));
        if (m_entities[static_cast<size_t>(e)] == static_cast<Entity>(Language::UInt32(-1))) return Language::StatusResult<Transform>::Err(Language::Err(Language::StatusDomain::Memory(), Language::StatusCode::NotFound));
        Language::USize idx = static_cast<Language::USize>(e) * static_cast<Language::USize>(3);
        Transform t{ m_positions[static_cast<size_t>(idx + 0)], m_positions[static_cast<size_t>(idx + 1)], m_positions[static_cast<size_t>(idx + 2)] };
        return Language::StatusResult<Transform>::Ok(t);
    }

    bool SceneWorld::SetParent(Entity child, Entity parent) noexcept {
        if (static_cast<Language::USize>(child) >= m_count) return false;
        if (static_cast<Language::USize>(parent) >= m_count) return false;
        if (child == parent) return false;
        m_parents[static_cast<size_t>(child)] = parent;
        return true;
    }

    bool SceneWorld::BindMesh(Entity e, Language::UInt32 h) noexcept {
        if (static_cast<Language::USize>(e) >= m_count) return false;
        m_meshes[static_cast<size_t>(e)] = h; return true;
    }

    bool SceneWorld::BindMaterial(Entity e, Language::UInt32 h) noexcept {
        if (static_cast<Language::USize>(e) >= m_count) return false;
        m_materials[static_cast<size_t>(e)] = h; return true;
    }

    SceneView SceneWorld::GetView() const noexcept { return SceneView{ m_entities, m_positions, m_meshes, m_materials, m_count }; }


    Language::USize SceneWorld::Count() const noexcept { return m_count; }

    bool SceneWorld::AddTransformComponent(Entity e) noexcept {
        if (static_cast<Language::USize>(e) >= m_count) return false;
        m_flags[static_cast<size_t>(e)] |= static_cast<Language::UInt8>(1u);
        return true;
    }
    bool SceneWorld::RemoveTransformComponent(Entity e) noexcept {
        if (static_cast<Language::USize>(e) >= m_count) return false;
        m_flags[static_cast<size_t>(e)] &= static_cast<Language::UInt8>(~static_cast<Language::UInt8>(1u));
        return true;
    }
    bool SceneWorld::HasTransform(Entity e) const noexcept {
        if (static_cast<Language::USize>(e) >= m_count) return false;
        return (m_flags[static_cast<size_t>(e)] & static_cast<Language::UInt8>(1u)) != static_cast<Language::UInt8>(0);
    }
}