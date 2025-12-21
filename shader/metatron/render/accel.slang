module accel;
import metatron.resource;

namespace mtt::accel {
    public struct Transform_Tag {
        stl::Tag tag;

        public func get() -> math::Transform {
            let arr = Ptr<math::Transform>(global.vectors[tag.storage()]);
            return arr[tag.index()];
        }
    }

    struct Divider {
        // tag<shape::Shape> shape;
        // tag<media::Medium> int_medium;
        // tag<media::Medium> ext_medium;
        // tag<material::Material> material;
        // tag<math::Transform> local_to_render;
        // tag<math::Transform> int_to_render;
        // tag<math::Transform> ext_to_render;
        u32 shape;
        u32 int_medium;
        u32 ext_medium;
        u32 material;
        Transform_Tag local_to_render;
        Transform_Tag int_to_render;
        Transform_Tag ext_to_render;
    }
}
