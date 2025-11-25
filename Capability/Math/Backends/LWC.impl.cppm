export module Math:LWC;
import :Storage;
namespace Math {
    Vector3 LWCConverter::SubAndCast(const DVector3& world_pos, const DVector3& cam_pos) noexcept {
        DVector3 d{ world_pos.x - cam_pos.x, world_pos.y - cam_pos.y, world_pos.z - cam_pos.z };
        return Vector3{ static_cast<Scalar>(d.x), static_cast<Scalar>(d.y), static_cast<Scalar>(d.z) };
    }
}
