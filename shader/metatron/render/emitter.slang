module emitter;
__include emitter.uniform;
import metatron.resource;

#define DISPATCH_V(method, ...) \
    if (idx.type() == 0) return tag<Uniform_Emitter>(idx).get().method(__VA_ARGS__);

#define DISPATCH_R(method, ...) DISPATCH_V(method, __VA_ARGS__) else return {};

namespace mtt::emitter {
    public struct Interaction {
        public light::Light light;
        public math::proxy::Transform local_to_render;
        public f32 pdf;
    }

    public struct Emitter {
        tag<byte> idx;
        public func sample(math::Context ctx, f32 u) -> Optional<Interaction> { DISPATCH_R(sample, ctx, u) }
        public func sample_infinite(math::Context ctx, f32 u) -> Optional<Interaction> { DISPATCH_R(sample_infinite, ctx, u) }
    }
}

#undef DISPATCH_V
#undef DISPATCH_R
