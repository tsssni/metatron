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
            return {};
        }

        public func sample(math::Context ctx, fv2 u, u32 idx) -> Optional<Interaction> {
            return {};
        }

        public func query(math::Ray r, u32 idx) -> Optional<fv4> {
            // TODO: software rt
            return none;
        }

        func local_to_shear(math::Ray r, u32 ri, fv3 rt, fv3 x) -> fv3 {
            var y = x - r.o;
            math::swap(y[2], y[ri]);
            let z = y[2] * rt;
            y[2] = 0.f;
            return y + z;
        };
    }
}

