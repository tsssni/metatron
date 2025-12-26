module monte_carlo;
__include monte_carlo.volume_path;
import accel;
import emitter;
import sampler;
import metatron.resource;

namespace mtt::monte_carlo {
    public struct Context {
        accel::Acceleration accel;
        emitter::Emitter emitter;
        sampler::Sampler sampler;
        fv4 lambda;
        math::Ray_Differential ray_differential;
        math::Ray_Differential default_differential;
        math::Transform render_to_camera;
        uzv2 pixel;
        usize sample_index;
        usize max_depth;
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
