module emitter;
__include emitter.uniform;
import metatron.resource;

namespace mtt::emitter {
    public struct Interaction {
        public light::Light_Tag light;
        public tag<math::Transform> local_to_render;
        public f32 pdf;
    }

    public interface Emitter {
        public func sample(math::Context ctx, f32 u) -> Optional<Interaction>;
        public func sample_infinite(math::Context ctx, f32 u) -> Optional<Interaction>;
    }

    public struct Emitter_Union {
        Uniform_Emitter uniform;
        u32 idx;

        public func get() -> Emitter {
            if (idx == 0) return uniform;
            else return {};
        }
    }
}
