module;
export module Engine.Scene:World;

import Lang;
import Cap.Memory;
import :Types;

export namespace Engine::Scene {
    class SceneWorld {
    public:
        SceneWorld(Cap::IAllocator* alloc) noexcept;
        Entity CreateEntity() noexcept;
        bool DestroyEntity(Entity e) noexcept;
        bool SetTransform(Entity e, const Transform& t) noexcept;
        StatusResult<Transform> GetTransform(Entity e) const noexcept;
        bool SetParent(Entity child, Entity parent) noexcept;
        bool AddTransformComponent(Entity e) noexcept;
        bool RemoveTransformComponent(Entity e) noexcept;
        bool HasTransform(Entity e) const noexcept;
        bool BindMesh(Entity e, UInt32 h) noexcept;
        bool BindMaterial(Entity e, UInt32 h) noexcept;
        SceneView GetView() const noexcept;
        USize Count() const noexcept;
    private:
        Cap::IAllocator* m_alloc{};
        Entity* m_entities{};
        float* m_positions{};
        UInt32* m_meshes{};
        UInt32* m_materials{};
        UInt8* m_flags{};
        Entity* m_parents{};
        USize m_count{0};
        USize m_cap{0};
    };
}