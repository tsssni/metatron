implementing material;

namespace mtt::material {
    public struct Interface_Material: Material {
        u32 padding = 0u;

        public func sample(
            math::Context ctx,
            texture::Coordinate coord
        ) -> Optional<Interaction> {
            return Interaction(
                bsdf::Bsdf_Union(bsdf::Interface_Bsdf()),
                fv4(0.f),
                {0.f, 0.f, 1.f},
                false,
            );
        }

        public func flags() -> Flags {
            return Flags::interface;
        }
    }
}
