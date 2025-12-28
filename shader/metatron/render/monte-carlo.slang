module monte_carlo;
__include monte_carlo.volume_path;
import accel;
import emitter;
import sampler;
import metatron.resource;

namespace mtt::monte_carlo {
    public struct Context {
        public accel::Acceleration_Tag accel = {};
        public emitter::Emitter_Tag emitter = {};
        public sampler::Proxy_Sampler sampler = {};
        public fv4 lambda = 0;
        public math::Ray_Differential ray_differential = {};
        public math::Ray_Differential default_differential = {};
        public math::Transform render_to_camera = {};
        public uv2 pixel = 0;
        public u32 sample_index = 0;
        public u32 max_depth = 0;
    }

    public interface Integrator {
        public func sample(inout Context ctx) -> Optional<spectra::Stochastic_Spectrum>;
    }

    public struct Integrator_Tag: Integrator {
        tag<byte> idx;

        public func sample(inout Context ctx) -> Optional<spectra::Stochastic_Spectrum> {
            if (idx.type() == 0) return tag<Volume_Path_Integrator>().get().sample(ctx);
            else return {};
        }
    }
}
