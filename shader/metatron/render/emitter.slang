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

    public struct Emitter_Tag: Emitter {
        tag<byte> idx;

        public func sample(math::Context ctx, f32 u) -> Optional<Interaction> {
            if (idx.type() == 0) return tag<Uniform_Emitter>(idx).get().sample(ctx, u);
            else return {};
        }

        public func sample_infinite(math::Context ctx, f32 u) -> Optional<Interaction> {
            if (idx.type() == 0) return tag<Uniform_Emitter>(idx).get().sample_infinite(ctx, u);
            else return {};
        }
    }
}
