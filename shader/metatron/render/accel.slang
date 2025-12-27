module accel;
__include accel.hwbvh;
__include accel.lbvh;
import metatron.resource;

namespace mtt::accel {
    public struct Divider {
        public shape::Shape_Tag shape = {};
        public media::Medium_Tag int_medium = {};
        public media::Medium_Tag ext_medium = {};
        public material::Material_Tag material = {};
        public tag<math::Transform> local_to_render = {};
        public tag<math::Transform> int_to_render = {};
        public tag<math::Transform> ext_to_render = {};
    }

    public struct Interaction {
        public Divider divider = {};
        public u32 primitive = 0;
        public Optional<shape::Interaction> intr_opt = {};
    };

    public interface Acceleration {
        public func trace(math::Ray r, fv3 n) -> Optional<Interaction>;
    }

    public struct Acceleration_Union: Acceleration {
        LBVH lbvh;
        HWBVH hwbvh;
        u32 idx;

        public func trace(math::Ray r, fv3 n) -> Optional<Interaction> {
            if (idx == 0) return lbvh.trace(r, n);
            else if (idx == 1) return hwbvh.trace(r, n);
            else return {};
        }
    }
}
