module;
export module Engine.Scene:World;

import Language;
import Foundation.Memory;
import :Types;

export namespace Engine::Scene {
    class SceneWorld {
    public:
        SceneWorld(::Foundation::Memory::IAllocator* alloc) noexcept;
        Entity CreateEntity() noexcept;
        bool DestroyEntity(Entity e) noexcept;
        bool SetTransform(Entity e, const Transform& t) noexcept;
        Language::StatusResult<Transform> GetTransform(Entity e) const noexcept;
        bool SetParent(Entity child, Entity parent) noexcept;
        bool AddTransformComponent(Entity e) noexcept;
        bool RemoveTransformComponent(Entity e) noexcept;
        bool HasTransform(Entity e) const noexcept;
        bool BindMesh(Entity e, Language::UInt32 h) noexcept;
        bool BindMaterial(Entity e, Language::UInt32 h) noexcept;
        SceneView GetView() const noexcept;
        Language::USize Count() const noexcept;
    private:
        ::Foundation::Memory::IAllocator* m_alloc{};
        Entity* m_entities{};
        float* m_positions{};
        Language::UInt32* m_meshes{};
        Language::UInt32* m_materials{};
        Language::UInt8* m_flags{};
        Entity* m_parents{};
        Language::USize m_count{0};
        Language::USize m_cap{0};
    };
}