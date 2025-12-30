implementing photo;

namespace mtt::photo {
    public struct Thin_Lens: Lens {
        public f32 aperture;
        public f32 focal_length;
        public f32 focus_distance;
        public f32 focal_distance;

        public func sample(fv2 o, fv2 u) -> Optional<lens::Interaction> {
            let center = fv3(0.f);
            let focused = fv3(-o, focal_distance) * focus_distance / focal_distance;
            
            let radius = focal_distance / aperture / 2.f;
            let distr: math::Unifrom_Disk_Distribution;
            let lens_p = fv3(distr.sample(u) * radius, 0);
            let direction = normalize(focused - lens_p);

            var intr: lens::Interaction;
            intr.r = math::Ray(lens_p, direction);
            intr.pdf = 1.f;
            return intr;
        }
    }
}
