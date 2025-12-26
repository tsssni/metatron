implementing photo;

namespace mtt::photo {
    public interface Lens {
        public func sample(fv2 o, fv2 u) -> Optional<lens::Interaction>;
    }

    public struct Lens_Union: Lens {
        Pinhole_Lens pinhole;
        Thin_Lens thin;
        u32 idx;

        public func sample(fv2 o, fv2 u) -> Optional<lens::Interaction> {
            if (idx == 0) return pinhole.sample(o, u);
            else if (idx == 1) return thin.sample(o, u);
            else return {};
        }
    }
}
