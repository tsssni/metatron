module monte_carlo;
import accel;
import emitter;
import sampler;
import metatron.resource;

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
}
