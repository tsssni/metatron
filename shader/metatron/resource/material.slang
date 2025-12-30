module material;
__include material.interface;
__include material.physical;
import bsdf;
import spectra;
import texture;
import metatron.core;

namespace mtt::material {
    public struct Interaction {
        public bsdf::Bsdf_Union bsdf;
        public fv4 emission;
        public fv3 normal = {0.f};
        public bool degraded = false;
    }

    public enum Flags {
        interface = 1 << 0,
        emissive = 1 << 1,
    }

    public interface Material {
        public func sample(
            math::Context ctx,
            texture::Coordinate coord
        ) -> Optional<Interaction>;
        public func flags() -> Flags;
    }

    public struct Material_Tag: Material {
        tag<byte> idx;

        public func sample(
            math::Context ctx,
            texture::Coordinate coord
        ) -> Optional<Interaction> {
            if (idx.type() == 0) return tag<Physical_Material>(idx).get().sample(ctx, coord);
            else if (idx.type() == 1) return tag<Interface_Material>(idx).get().sample(ctx, coord);
            else return {};
        }

        public func flags() -> Flags {
            if (idx.type() == 0) return tag<Physical_Material>(idx).get().flags();
            else if (idx.type() == 1) return tag<Interface_Material>(idx).get().flags();
            else return {};
        }
    }
}
