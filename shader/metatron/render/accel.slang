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
        public Optional<Interaction> operator()(math::Ray r, fv3 n);
    }

    public struct Acceleration_Tag: Acceleration {
        tag<byte> idx;

        public Optional<Interaction> operator()(math::Ray r, fv3 n) {
            if (idx.type() == 0) return tag<LBVH>(idx).get()(r, n);
            else if (idx.type() == 1) return tag<HWBVH>(idx).get()(r, n);
            else return {};
        }
    }
}
