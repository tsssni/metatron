module monte_carlo;
__include monte_carlo.volume_path;
import accel;
import emitter;
import sampler;
import metatron.resource;

namespace mtt::monte_carlo {
    public struct Context {
        public accel::Acceleration_Union accel = {};
        public emitter::Emitter_Union emitter = {};
        public sampler::Sampler_Union sampler = {};
        public fv4 lambda = 0;
        public math::Ray_Differential ray_differential = {};
        public math::Ray_Differential default_differential = {};
        public math::Transform render_to_camera = {};
        public uzv2 pixel = 0;
        public usize sample_index = 0;
        public usize max_depth = 0;
    }

    public interface Integrator {
        public func sample(Context ctx) -> Optional<spectra::Stochastic_Spectrum>;
    }

    public struct Integrator_Union: Integrator {
        Volume_Path_Integrator volume_path;
        u32 idx;

        public func sample(Context ctx) -> Optional<spectra::Stochastic_Spectrum> {
            if (idx == 0) return volume_path.sample(ctx);
            else return {};
        }
    }
}
