implementing shape;

namespace mtt::shape {
    public struct Mesh: Shape {
        buf<uv3> indices;

        buf<fv3> vertices;
        buf<fv3> normals;
        buf<fv2> uvs;

        buf<fv3> dpdu;
        buf<fv3> dpdv;
        buf<fv3> dndu;
        buf<fv3> dndv;

        public func size() -> u32 { return indices.size(); }

        public func bounding_box(math::Transform t, u32 idx) -> math::Bounding_Box {
            let prim = indices[idx];
            let v = Array<fv4, 3>(
                t * fv4(vertices[prim[0]], 1.f),
                t * fv4(vertices[prim[1]], 1.f),
                t * fv4(vertices[prim[2]], 1.f)
            );
            let p_min = min(min(v[0], v[1]), v[2]);
            let p_max = max(max(v[0], v[1]), v[2]);
            return {p_min.xyz, p_max.xyz};
        }

        public Optional<Interaction> operator()(
            math::Ray r, fv3 np,
            fv4 pos, u32 idx
        ) {
            let bary = pos.xyz;
            let t = pos.w;
            let pdf = this.pdf(r, np, idx);
            let p = blerp(vertices, bary, idx);
            let n = blerp(normals, bary, idx);
            let tn = math::gram_schmidt(dpdu[idx], n);
            let bn = cross(tn, n);
            let uv = blerp(uvs, bary, idx);

            return Interaction(
                p, n, tn, bn, uv, t, pdf,
                dpdu[idx], dpdv[idx],
                dndu[idx], dndv[idx],
            );
        }

        public func sample(math::Context ctx, fv2 u, u32 idx) -> Optional<Interaction> {
            let prim = indices[idx];
            struct Validator {
                bool operator()(fv3 v) { return dot(v, v) >= math::epsilon; }
            }
            let validate_vector: Validator;

            let a = normalize(vertices[prim[0]] - ctx.r.o);
            let b = normalize(vertices[prim[1]] - ctx.r.o);
            let c = normalize(vertices[prim[2]] - ctx.r.o);
            if (false
            || validate_vector(a)
            || validate_vector(b)
            || validate_vector(c)) return {};

            let n_ab = normalize(cross(b, a));
            let n_bc = normalize(cross(c, b));
            let n_ca = normalize(cross(a, c));
            if (false
            || validate_vector(n_ab)
            || validate_vector(n_bc)
            || validate_vector(n_ca)) return {};

            let alpha = math::angle(n_ab, -n_ca);
            let beta = math::angle(n_bc, -n_ab);
            let gamma = math::angle(n_ca, -n_bc);

            let A_pi = alpha + beta + gamma;
            let A_1_pi = lerp(math::pi, A_pi, u[0]);

            let phi = A_1_pi - alpha;
            let cos_a = cos(alpha);
            let sin_a = sin(alpha);
            let cos_p = cos(phi);
            let sin_p = sin(phi);

            let k_1 = cos_p + cos_a;
            let k_2 = sin_p - sin_a * dot(a, b);
            let cos_ac1 = math::guarded_div(
                k_2 + (k_2 * cos_p - k_1 * sin_p) * cos_a,
                (k_2 * sin_p + k_1 * cos_p) * sin_a
            );
            let sin_ac1 = math::sqrt(1.f - cos_ac1 * cos_ac1);
            let c_1 = cos_ac1 * a + sin_ac1 * normalize(math::gram_schmidt(c, a));

            let cos_bc1 = dot(b, c_1);
            let cos_bc2 = 1.f - u[1] * (1.f - cos_bc1);
            let sin_bc2 = math::sqrt(1.f - cos_bc2 * cos_bc2);
            let d = cos_bc2 * b + sin_bc2 * normalize(math::gram_schmidt(c_1, b));

            let v = Array<fv3, 3>(
                vertices[prim[0]],
                vertices[prim[1]],
                vertices[prim[2]],
            );
            let cramer_opt = math::cramer(
                transpose(fm33(-d, v[1] - v[0], v[2] - v[0])),
                ctx.r.o - v[0]
            );
            if (cramer_opt == none) return {};
            let cramer = cramer_opt.value;

            let t = cramer.x;
            var b_1 = cramer.y;
            var b_2 = cramer.z;
            if (b_1 + b_2 > 1.f) {
                b_1 /= (b_1 + b_2);
                b_2 /= (b_1 + b_2);
            }
            let bary = fv3(1.f - b_1 - b_2, b_1, b_2);

            let pdf = this.pdf({ctx.r.o, d}, ctx.n, idx);
            let p = ctx.r.o + t * d;
            let n = blerp(normals, bary, idx);
            let tn = math::gram_schmidt(dpdu[idx], n);
            let bn = cross(tn, n);
            let uv = blerp(uvs, bary, idx);

            return Interaction(
                p, n, tn, bn, uv, t, pdf,
                dpdu[idx], dpdv[idx],
                dndu[idx], dndv[idx],
            );
        }

        public func query(math::Ray r, u32 idx) -> Optional<fv4> {
            // TODO: software rt
            return none;
        }

        func blerp<let n: u32>(
            buf<vector<f32, n>> x,
            fv3 b,
            u32 idx
        ) -> vector<f32, n> {
            if (x.empty()) return {};
            let prim = indices[idx];
            var y = vector<f32, n>(0);
            for (var i = 0; i < 3; ++i)
                y += x[prim[i]] * b[i];
            return y;
        }

        func pdf(
            math::Ray r, fv3 np, u32 idx
        ) -> f32 {
            let prim = indices[idx];
            let a = normalize(vertices[prim[0]] - r.o);
            let b = normalize(vertices[prim[1]] - r.o);
            let c = normalize(vertices[prim[2]] - r.o);

            let n_ab = normalize(cross(b, a));
            let n_bc = normalize(cross(c, b));
            let n_ca = normalize(cross(a, c));

            let alpha = math::angle(n_ab, -n_ca);
            let beta = math::angle(n_bc, -n_ab);
            let gamma = math::angle(n_ca, -n_bc);

            let c_2 = normalize(r.d);
            var c_1 = normalize(cross(cross(b, c_2), cross(c, a)));
            if (dot(c_1, a + c) < 0.f) c_1 *= -1.f;

            var u = fv2(0);
            var pdf = 1.f;
            u[1] = math::guarded_div(1.f - dot(b, c_2), (1.f - dot(b, c_1)));
            pdf = math::guarded_div(pdf, 1.f - dot(b, c_1));
            if (dot(a, c_1) > 0.99999847691f) { // 0.1 degrees
                u[0] = 0.f;
            } else {
                let n_bc1 = normalize(cross(c_1, b));
                let n_c1a = normalize(cross(a, c_1));
                let A = alpha + beta + gamma - math::pi;
                pdf = math::guarded_div(pdf, A);

                if (length(n_bc1) < math::epsilon || length(n_c1a) < math::epsilon) {
                    u = {0.5f};
                } else {
                    let A_1 = alpha + math::angle(n_bc1, -n_ab) + math::angle(n_c1a, -n_bc1) - math::pi;
                    u[1] = math::guarded_div(A_1, A);
                }
            }

            if (!all(np == fv3(0.f))) {
                let distr = math::Bilinear_Distribution(
                    dot(np, b), dot(np, a),
                    dot(np, b), dot(np, c),
                );
                pdf *= distr.pdf(u);
            }
            return pdf;
        }
    }
}

