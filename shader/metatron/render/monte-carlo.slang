module monte_carlo;
__include monte_carlo.radiative;
__include monte_carlo.restir;
import accel;
import emitter;
import sampler;
import metatron.resource;

#define DISPATCH_V(method, ...) \
    if (idx.type() == 0) return tag<Radiative_Integrator>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Restir_Integrator>(idx).get().method(__VA_ARGS__);

#define DISPATCH_R(method, ...) DISPATCH_V(method, __VA_ARGS__) else return {};

namespace mtt::monte_carlo {
    public struct Context {
        public accel::Acceleration accel = {};
        public emitter::Emitter emitter = {};
        public sampler::proxy::Sampler sampler = {};
        public fv4 lambda = 0;
        public math::Ray_Differential ray_differential = {};
        public math::Ray_Differential default_differential = {};
        public math::Transform render_to_camera = {};
        public uv2 pixel = 0;
        public u32 sample_index = 0;
        public u32 max_depth = 0;
    }

    public struct Integrator {
        tag<byte> idx;
        public func sample(Context ctx) -> Optional<spectra::Stochastic_Spectrum> { DISPATCH_R(sample, ctx) }
    }
}

#undef DISPATCH_V
#undef DISPATCH_R
