implementing media;

namespace mtt::media {
    #define update_maj(t_max) update_majorant(\
        t_max, r, direction, t_boundary, sigma_t,\
        t_cell, cell, offset, density_maj, sigma_maj, distr\
    )
    #define update_T(t) update_transmittance(\
        t, sigma_maj, t_transmitted, t_boundary, r, transmittance\
    )

    public struct Heterogeneous_Medium: Medium {
        Phase phase;
        spectra::Spectrum_Tag sigma_a;
        spectra::Spectrum_Tag sigma_s;
        spectra::Spectrum_Tag sigma_e;
        volume::Volume_Tag majorant;
        volume::Volume_Tag density;
        f32 density_scale;

        public func sample(math::Context ctx, f32 t_max, f32 u) -> Optional<Interaction> {
            let sigma_a = (ctx.lambda & this.sigma_a) * density_scale;
            let sigma_s = (ctx.lambda & this.sigma_s) * density_scale;
            let sigma_t = sigma_a + sigma_s;
            var sigma_maj = sigma_t;
            var transmittance = fv4(1.f);
            var density_maj = 0.f;
            var distr = math::Exponential_Distribution(0.f);

            var r = ctx.r;
            var cell = majorant.to_index(r.o); var offset = iv3(0);
            var direction = sign(r.d);

            var t_cell = t_max;
            var t_boundary = t_max;
            var t_transmitted = 0.f;

            if (!majorant.inside(cell)) {
                var t_enter = 0.f; var t_exit = 0.f;
                let hit_opt = math::hit(r, majorant.bounding_box());
                if (hit_opt.hasValue) {
                    t_enter = hit_opt.value.x;
                    t_exit = hit_opt.value.y;
                }

                r.o = r.o + t_enter * r.d;
                cell = clamp(majorant.to_index(r.o), iv3(0), majorant.dimensions() - 1);
            }
            update_maj(t_max);

            while (true) {
                let t_u = distr.sample(u);
                if (true
                && t_boundary <= t_cell
                && (density_maj < math::epsilon || t_u >= t_boundary)) {
                    update_T(t_boundary);
                    return Interaction(
                        phase.to_phase(),
                        r.o,
                        t_max,
                        transmittance,
                        {}, {}, {}, {}, {},
                    );
                } else if (true
                && t_boundary > t_cell
                && (density_maj < math::epsilon || t_u >= t_cell)) {
                    update_T(t_cell);
                    update_maj(t_boundary);
                } else {
                    update_T(t_u);
                    let spectra_pdf = sigma_maj * transmittance;
                    let density = this.density(r.o);

                    return Interaction(
                        phase.to_phase(),
                        r.o,
                        t_transmitted,
                        transmittance,
                        density * sigma_a,
                        density * sigma_s,
                        max(sigma_maj - density * sigma_t, fv4(0.f)),
                        sigma_maj,
                        density * (!sigma_e.empty() ? (ctx.lambda & sigma_e) : fv4(0.f)),
                    );
                }
            }
            return {};
        }

        func update_majorant(
            f32 t_max, math::Ray r, iv3 direction,
            f32 t_boundary, fv4 sigma_t,
            inout f32 t_cell, inout iv3 cell, inout iv3 offset,
            inout f32 density_maj, inout fv4 sigma_maj,
            inout math::Exponential_Distribution distr,
        ) -> void {
            let inside = majorant.inside(cell);
            let next_inside = majorant.inside(cell + offset);
            if (inside && !next_inside) {
                t_cell = t_boundary;
                return;
            }

            let bbox = majorant.bounding_box(cell + offset);
            let hit_opt = math::hitvi(r, bbox);
            var t_enter = t_boundary; var t_next = t_boundary;
            var i_enter = 0; var i_next = 0;
            if (hit_opt.hasValue) {
                let hitted = hit_opt.value;
                t_enter = hitted._0; t_next = hitted._1;
                i_enter = hitted._2; i_next = hitted._3;
            }

            t_cell = t_next;
            cell += offset;
            offset = direction * iv3(i_next == 0, i_next == 1, i_next == 2);

            density_maj = majorant[cell];
            sigma_maj = density_maj * sigma_t;
            distr = math::Exponential_Distribution(sigma_maj[0]);
        }

        func update_transmittance(
            f32 t, fv4 sigma_maj,
            inout f32 t_transmitted, inout f32 t_boundary,
            inout math::Ray r, inout fv4 transmittance,
        ) -> void {
            t_transmitted += t;
            t_boundary -= t;
            r.o += t * r.d;
            transmittance *= exp(-sigma_maj * t);
        }
    }
}
