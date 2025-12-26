implementing photo;

namespace mtt::photo {
    public struct Interaction {
        public math::Ray_Differential ray_differential;
        public math::Ray_Differential default_differential;
        public f32 pdf;
    }

    public struct Camera {
        public func sample<T: Lens>(
            T lens,
            fv2 pos,
            fv2 dxdy,
            fv2 u
        ) -> Optional<Interaction> {
            var intr: Interaction;

            {
                var ray: math::Ray_Differential;
                let r_pos = pos;
                let rx_pos = r_pos + fv2(dxdy[0], 0.f);
                let ry_pos = r_pos + fv2(0.f, dxdy[1]);

                let r_intr = lens.sample(r_pos, u); if (r_intr == none) return none;
                let rx_intr = lens.sample(rx_pos, u); if (rx_intr == none) return none;
                let ry_intr = lens.sample(ry_pos, u); if (ry_intr == none) return none;

                ray.differentiable = true;
                ray.r = r_intr.value.r;
                ray.rx = rx_intr.value.r;
                ray.ry = ry_intr.value.r;
                intr.pdf = r_intr.value.pdf;
                intr.ray_differential = ray;
            }

            {
                var ray: math::Ray_Differential;
                let r_pos = fv2(0);
                let rx_pos = r_pos + fv2(dxdy[0], 0.f);
                let ry_pos = r_pos + fv2(0.f, dxdy[1]);

                let r_intr = lens.sample(r_pos, 0); if (r_intr == none) return none;
                let rx_intr = lens.sample(rx_pos, 0); if (rx_intr == none) return none;
                let ry_intr = lens.sample(ry_pos, 0); if (ry_intr == none) return none;

                ray.differentiable = false;
                ray.r = r_intr.value.r;
                ray.rx = rx_intr.value.r;
                ray.ry = ry_intr.value.r;
                intr.default_differential = ray;
            }

            return intr;
        }
    }
}
