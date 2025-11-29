module;
export module Engine.Animation:Player;

import Lang;
import :Types;

export namespace Engine::Animation {
    class AnimationPlayer {
    public:
        bool BindSkeleton(const Skeleton* skel) noexcept { m_skeleton = skel; return skel != nullptr; }
        bool Play(ClipId id) noexcept { m_current = id; m_time = 0.0f; return true; }
        void Update(float dt) noexcept { m_time += dt; }
        Pose Evaluate() const noexcept { return Pose{ m_pose, m_poseCount }; }
    private:
        const Skeleton* m_skeleton{nullptr};
        ClipId m_current{0};
        float m_time{0.0f};
        const JointMatrix* m_pose{nullptr};
        Language::USize m_poseCount{0};
    };
}