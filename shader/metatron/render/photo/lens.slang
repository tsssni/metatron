implementing photo;

namespace mtt::photo {
    public interface Lens {
        public func sample(fv2 o, fv2 u) -> Optional<lens::Interaction>;
    }

    public struct Lens_Tag: Lens {
        tag<byte> idx;

        public func sample(fv2 o, fv2 u) -> Optional<lens::Interaction> {
            if (idx.type() == 0) return tag<Pinhole_Lens>(idx).get().sample(o, u);
            else if (idx.type() == 1) return tag<Thin_Lens>(idx).get().sample(o, u);
            else return {};
        }
    }
}
