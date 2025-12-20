implementing photo;

namespace mtt::photo {
    public struct Pinhole_Lens: Lens {
        public f32 focal_distance = 0.035f;
        public func sample(fv2 o, fv2 u) -> Optional<lens::Interaction> {
            let p = fv3(0.0, 0.0, focal_distance);
            var intr: lens::Interaction;
            intr.r = math::Ray(fv3(0), fv3(-o, focal_distance));
            intr.pdf = 1.0;
            return intr;
        }
    }
}
