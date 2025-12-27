implementing shape;

namespace mtt::shape {
    public struct Sphere: Shape {
        f32 padding = 0.f;

        public func size() -> u32 { return 1; }

        public func bounding_box(math::Transform t, u32 idx = 0) -> math::Bounding_Box {
            let bbox = math::Bounding_Box({-1.f}, {1.f});
            return t * bbox;
        }

        public Optional<Interaction> operator()(
            math::Ray r, fv3 np,
            fv4 pos, u32 idx = 0
        ) {
            let t = pos.w;
            let p = r.o + t * r.d;
            let n = p;
            let s = math::cartesian_to_unit_spherical(p);
            let theta = s[0];
            let phi = s[1];
            let uv = fv2(theta / math::pi, phi / (2.f * math::pi));

            let dpdu = fv3(
                cos(theta) * cos(phi),
                -sin(theta),
                cos(theta) * sin(phi),
            ) / math::pi;
            let dpdv = fv3(
                -sin(theta) * sin(phi),
                0.f,
                sin(theta) * cos(phi),
            ) / (2.f * math::pi);
            let dndu = dpdu;
            let dndv = dpdv;
            let tn = math::gram_schmidt(normalize(dpdu), n);
            let bn = cross(tn, n);

            let pdf = length(r.o) < 1.f
            ? math::Sphere_Distribution().pdf()
            : math::Cone_Distribution(math::sqrt(1.f - 1.f / dot(r.o, r.o))).pdf();

            return Interaction(p, n, tn, bn, uv, t, pdf, dpdu, dpdv, dndu, dndv);
        }

        public func sample(math::Context ctx, fv2 u, u32 idx) -> Optional<Interaction> {
            let d = length(ctx.r.o);
            var r: math::Ray;
            if (d < 1.f) {
                let distr = math::Sphere_Distribution();
                let p = distr.sample(u);
                let dir = normalize(p - ctx.r.o);
                r = math::Ray(ctx.r.o, dir);
            } else {
                let cos_theta_max = math::sqrt(1.f - 1.f / (d * d));
                let distr = math::Cone_Distribution(cos_theta_max);
                let dir = normalize(-ctx.r.o);
                var sdir = distr.sample(u);
                let rot = fq::from_rotation_between({0.f, 0.f, 1.f}, dir);
                sdir = math::rotate(fv4(sdir, 0.f), rot).xyz;
                r = math::Ray(ctx.r.o, sdir);
            }
            let pos = query(r);
            if (pos == none) return none;
            return this(r, ctx.n, pos.value);
        }

        public func query(math::Ray r, u32 idx = 0) -> Optional<fv4> {
            let a = dot(r.d, r.d);
            let b = dot(r.o, r.d) * 2.f;
            let c = dot(r.o, r.o) - 1.f;

            let delta = b * b - 4.f * a * c;
            if (delta < 0) return none;

            let x0 = (-b - math::sqrt(delta)) / (2.f * a);
            let x1 = (-b + math::sqrt(delta)) / (2.f * a);
            if (x1 < 0) return none;
            return fv4(0, 0, 0, x0 < 0.f ? x1 : x0);
        }
    }
}
