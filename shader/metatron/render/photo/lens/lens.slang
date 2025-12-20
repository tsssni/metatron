implementing photo;

namespace mtt::photo {
    public interface Lens {
        public func sample(fv2 o, fv2 u) -> Optional<lens::Interaction>;
    }

    public struct Lens_Union {
        Pinhole_Lens pinhole;
        Thin_Lens thin;
        u32 idx;

        public func get() -> Lens {
            if (idx == 0) return pinhole;
            else if (idx == 1) return thin;
            else return {};
        }
    }
}
