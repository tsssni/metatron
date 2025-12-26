implementing emitter;

namespace mtt::emitter {
    public struct Uniform_Emitter: Emitter {
        struct Primitive {
            light::Light_Tag light;
            tag<math::Transform> local_to_render;
        }
        buf<Primitive> prims;
        buf<Primitive> inf_prims;

        public func sample(math::Context ctx, f32 u) -> Optional<Interaction> {
            if (prims.empty() && inf_prims.empty()) return {};
            let idx = clamp(u32(u * prims.size()), 0u, prims.size() - 1);
            let prim = prims[idx];
            return Interaction(
                prim.light,
                prim.local_to_render,
                math::guarded_div(1.f, f32(prims.size()))
            );
        }

        public func sample_infinite(math::Context ctx, f32 u) -> Optional<Interaction> {
            if (inf_prims.empty()) return {};
            let idx = clamp(u32(u * inf_prims.size()), 0u, inf_prims.size() - 1);
            let prim = inf_prims[idx];
            return Interaction(
                prim.light,
                prim.local_to_render,
                math::guarded_div(1.f, f32(inf_prims.size()))
            );
        }
    }
}
