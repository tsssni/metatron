module accel;
__include accel.hwbvh;
__include accel.lbvh;
import metatron.resource;

namespace mtt::accel {
    public struct Divider {
        // tag<shape::Shape> shape;
        // tag<media::Medium> int_medium;
        // tag<media::Medium> ext_medium;
        // tag<material::Material> material;
        // tag<math::Transform> local_to_render;
        // tag<math::Transform> int_to_render;
        // tag<math::Transform> ext_to_render;
        shape::Shape_Tag shape = {};
        u32 int_medium = 0;
        u32 ext_medium = 0;
        u32 material = 0;
        tag<math::Transform> local_to_render = {};
        tag<math::Transform> int_to_render = {};
        tag<math::Transform> ext_to_render = {};
    }

    public struct Interaction {
        public Divider divider = {};
        public u32 primitive = 0;
        public Optional<shape::Interaction> intr_opt = {};
    };

    public interface Acceleration {
        public func trace(math::Ray r, fv3 n) -> Optional<Interaction>;
    }

    public struct Acceleration_Union {
        LBVH lbvh;
        HWBVH hwbvh;
        u32 idx;

        public func get() -> Acceleration {
            if (idx == 0) return lbvh;
            else if (idx == 1) return hwbvh;
            else return {};
        }
    }
}
