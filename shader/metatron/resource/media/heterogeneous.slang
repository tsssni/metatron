implementing media;

namespace mtt::media {
    public struct Heterogeneous_Medium {
        Phase phase = {};
        spectra::Spectrum_Tag sigma_a = {};
        spectra::Spectrum_Tag sigma_s = {};
        spectra::Spectrum_Tag sigma_e = {};
        volume::Volume_Tag majorant = {};
        volume::Volume_Tag density = {};
        f32 density_scale = {};

        public struct Iterator {
            Heterogeneous_Medium medium = {};
            fv4 sigma_a = {};
            fv4 sigma_s = {};
            fv4 sigma_t = {};
            fv4 sigma_maj = {};

            fv4 lambda = {};
            fv4 transmittance = 1.f;
            f32 density_maj = 0.f;
            math::Exponential_Distribution distr = {0.f};

            math::Ray r = {};
            f32 u = 0.f;

            iv3 cell = {};
            iv3 offset = {};
            iv3 direction = {};

            f32 t_max = 0.f;
            f32 t_cell = 0.f;
            f32 t_boundary = 0.f;
            f32 t_transmitted = 0.f;

            __init() {}
            __init(Heterogeneous_Medium medium, math::Context ctx, f32 t_max) {
                this.medium = medium;
                this.t_max = t_max;
                this.lambda = ctx.lambda;

                sigma_a = (ctx.lambda & medium.sigma_a) * medium.density_scale;
                sigma_s = (ctx.lambda & medium.sigma_s) * medium.density_scale;
                sigma_t = sigma_a + sigma_s;
                sigma_maj = sigma_t;

                r = ctx.r;
                cell = medium.majorant.to_index(r.o);
                offset = iv3(0);
                direction = sign(r.d);

                t_cell = t_max;
                t_boundary = t_max;
                t_transmitted = 0.f;

                if (!medium.majorant.inside(cell)) {
                    var t_enter = 0.f; var t_exit = 0.f;
                    let hit_opt = math::hit(r, medium.majorant.bounding_box());
                    if (hit_opt.hasValue) {
                        t_enter = hit_opt.value.x;
                        t_exit = hit_opt.value.y;
                    }

                    r.o = r.o + t_enter * r.d;
                    cell = clamp(medium.majorant.to_index(r.o), iv3(0), medium.majorant.dimensions() - 1);
                }
                update_majorant(t_max);
            }

            [mutating] func update_majorant(f32 t_max) -> void {
                let inside = medium.majorant.inside(cell);
                let next_inside = medium.majorant.inside(cell + offset);
                if (inside && !next_inside) {
                    t_cell = t_boundary;
                    return;
                }

                let bbox = medium.majorant.bounding_box(cell + offset);
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

                density_maj = medium.majorant[cell];
                sigma_maj = density_maj * sigma_t;
                distr = math::Exponential_Distribution(sigma_maj[0]);
            }

            [mutating] func update_transmittance(f32 t) -> void {
                t_transmitted += t;
                t_boundary -= t;
                t_cell -= t;
                r.o += t * r.d;
                u = 1.f - (1.f - u) * exp(sigma_maj[0] * t);
                transmittance *= exp(-sigma_maj * t);
            }

            [mutating] public func march(f32 uu) -> Optional<Interaction> {
                transmittance = 1.f;
                t_transmitted = 0.f;
                u = uu;

                while (true) {
                    let t_u = distr.sample(u);
                    if (t_boundary <= t_cell
                    && (density_maj < math::epsilon || t_u >= t_boundary)) {
                        update_transmittance(t_boundary);
                        return Interaction(
                            medium.phase.to_phase(),
                            r.o,
                            t_max,
                            transmittance,
                            {}, {}, {}, {}, {},
                        );
                    } else if (true
                    && t_boundary > t_cell
                    && (density_maj < math::epsilon || t_u >= t_cell)) {
                        update_transmittance(t_cell);
                        update_majorant(t_boundary);
                    } else {
                        update_transmittance(t_u);
                        let spectra_pdf = sigma_maj * transmittance;
                        let density = medium.density(r.o);

                        return Interaction(
                            medium.phase.to_phase(),
                            r.o,
                            t_transmitted,
                            transmittance,
                            density * sigma_a,
                            density * sigma_s,
                            max(sigma_maj - density * sigma_t, fv4(0.f)),
                            sigma_maj,
                            density * (!medium.sigma_e.empty()
                            ? (lambda & medium.sigma_e) : fv4(0.f)),
                        );
                    }
                }
                return {};
            }
        }

        public func begin(math::Context ctx, f32 t_max) -> Iterator {
            return Iterator(this, ctx, t_max);
        }
    }
}
