module;
export module Engine.Animation:Types;

import Lang;
import Foundation.Math;

export namespace Engine::Animation {
    using JointIndex = UInt32;
    using ClipId = UInt32;
    struct Skeleton { JointIndex jointCount{0}; const JointIndex* parents{nullptr}; };
    struct Clip { ClipId id{0}; float duration{0.0f}; };
    using JointMatrix = ::Foundation::Math::Matrix4x4;
    struct Pose { const JointMatrix* joints{nullptr}; USize count{0}; };
}