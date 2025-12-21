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

        public func intersect(
            math::Ray r, fv3 np,
            fv4 pos, u32 idx
        ) -> Optional<Interaction> {
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
            return {};
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

