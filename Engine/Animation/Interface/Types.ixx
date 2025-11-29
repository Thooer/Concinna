module;
export module Engine.Animation:Types;

import Lang;
import Foundation.Math;

export namespace Engine::Animation {
    using JointIndex = Language::UInt32;
    using ClipId = Language::UInt32;
    struct Skeleton { JointIndex jointCount{0}; const JointIndex* parents{nullptr}; };
    struct Clip { ClipId id{0}; float duration{0.0f}; };
    using JointMatrix = ::Foundation::Math::Matrix4x4;
    struct Pose { const JointMatrix* joints{nullptr}; Language::USize count{0}; };
}