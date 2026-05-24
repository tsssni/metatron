module accel;
__include accel.hwbvh;
__include accel.lbvh;
__include accel.proxy;
import metatron.resource;

#define DISPATCH_OP(...) \
         if (idx.type() == 0) return tag<LBVH>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<HWBVH>(idx).get()(__VA_ARGS__); \
    else return {};

namespace mtt::accel {
    public struct Divider {
        public shape::Shape shape = {};
        public media::Medium int_medium = {};
        public media::Medium ext_medium = {};
        public material::Material material = {};
        public math::proxy::Transform local_to_render = {};
        public math::proxy::Transform int_to_render = {};
        public math::proxy::Transform ext_to_render = {};
    }

    public struct Interaction {
        public proxy::Divider divider = {};
        public u32 primitive = 0;
        public shape::Interaction intr = {};
        public bool valid = false;
    };

    public struct Acceleration {
        tag<byte> idx;
        public Optional<Interaction> operator()(math::Ray r, fv3 n) { DISPATCH_OP(r, n) }
    }
}

#undef DISPATCH_OP
