module material;
__include material.interface;
__include material.physical;
import bsdf;
import spectra;
import texture;
import metatron.core;

#define DISPATCH_V(method, ...) \
         if (idx.type() == 0) return tag<Physical_Material>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Interface_Material>(idx).get().method(__VA_ARGS__);

#define DISPATCH_R(method, ...) DISPATCH_V(method, __VA_ARGS__) else return {};

namespace mtt::material {
    public struct Interaction {
        public bsdf::Bsdf bsdf;
        public fv4 emission;
        public fv3 normal = {0.f};
        public bool degraded = false;
    }

    public enum Flags {
        interface = 1 << 0,
        emissive = 1 << 1,
    }

    public struct Material {
        tag<byte> idx;
        public func sample(math::Context ctx, texture::Coordinate coord) -> Optional<Interaction> { DISPATCH_R(sample, ctx, coord) }
        public func flags() -> Flags { DISPATCH_R(flags) }
    }
}

#undef DISPATCH_V
#undef DISPATCH_R
