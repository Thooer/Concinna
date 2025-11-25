module;
export module Math:LWC;
import Language;
import SIMD;
import :Storage;
import :Compute;
export namespace Math {
    struct LWCConverter {
        static Vector3 SubAndCast(const DVector3& world_pos, const DVector3& cam_pos) noexcept;

        template<USize BlockSize>
        static inline void BatchConvert(const SoAChunkDVector3<BlockSize>& world_positions,
                                        const DVector3& camera_position,
                                        SoAChunkVector3<BlockSize>& local_positions,
                                        USize count) noexcept {
            for (USize i = 0; i < count; ++i) {
                local_positions.xs[i] = static_cast<Scalar>(world_positions.xs[i] - camera_position.x);
                local_positions.ys[i] = static_cast<Scalar>(world_positions.ys[i] - camera_position.y);
                local_positions.zs[i] = static_cast<Scalar>(world_positions.zs[i] - camera_position.z);
            }
        }

        template<USize BlockSize>
        static inline void BatchConvertPacket4(const SoAChunkDVector3<BlockSize>& world_positions,
                                               const DVector3& camera_position,
                                               SoAChunkVector3<BlockSize>& local_positions,
                                               USize count) noexcept {
            DVector3Packet<4> cam{ 
                SIMD::Packet<double, 4>{ _mm256_set1_pd(camera_position.x) },
                SIMD::Packet<double, 4>{ _mm256_set1_pd(camera_position.y) },
                SIMD::Packet<double, 4>{ _mm256_set1_pd(camera_position.z) }
            };
            USize i = 0;
            for (; i + 4 <= count; i += 4) {
                auto world = LoadDVector3PacketUnaligned(world_positions, i);
                auto delta = DVector3Packet<4>::Sub(world, cam);
                auto local = ConvertDVector3ToVector3<4, 4>(delta);
                StorePacketUnaligned(local_positions, i, local);
            }
            for (; i < count; ++i) {
                local_positions.xs[i] = static_cast<Scalar>(world_positions.xs[i] - camera_position.x);
                local_positions.ys[i] = static_cast<Scalar>(world_positions.ys[i] - camera_position.y);
                local_positions.zs[i] = static_cast<Scalar>(world_positions.zs[i] - camera_position.z);
            }
        }
    };
}
