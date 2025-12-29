implementing material;

namespace mtt::material {
    public struct Physical_Material: Material {
        texture::Spectrum_Texture_Tag reflectance;
        texture::Spectrum_Texture_Tag eta;
        texture::Spectrum_Texture_Tag k;
        texture::Spectrum_Texture_Tag emission;

        texture::Vector_Texture_Tag alpha;
        texture::Vector_Texture_Tag alpha_u;
        texture::Vector_Texture_Tag alpha_v;
        texture::Vector_Texture_Tag normal;

        public func sample(
            math::Context ctx,
            texture::Coordinate coord
        ) -> Optional<Interaction> {
            var reflectance = guarded_sample(this.reflectance, fv4(2.f), coord, ctx.lambda);
            var eta = guarded_sample(this.eta, fv4(0.f), coord, ctx.lambda);
            var k = guarded_sample(this.k, fv4(0.f), coord, ctx.lambda);
            var emission = guarded_sample(this.emission, fv4(0.f), coord, ctx.lambda);

            var alpha_u = 0.f;
            var alpha_v = 0.f;
            if (!this.alpha.empty()) alpha_u = alpha_v = this.alpha(coord)[0];
            if (!this.alpha_u.empty()) alpha_u = guarded_sample(this.alpha_u, fv4(alpha_u), coord)[0];
            if (!this.alpha_v.empty()) alpha_v = guarded_sample(this.alpha_v, fv4(alpha_v), coord)[0];
            let normal = guarded_sample(this.normal, fv4(0.5f, 0.5f, 1.f, 0.f), coord);

            var lambda = ctx.lambda;
            let degraded = true
            && any(reflectance != saturate(reflectance))
            && any(eta != fv4(0.f)) && !math::constant(eta)
            && all(k == fv4(0.f));

            if (degraded) {
                lambda = fv4(lambda[0]);
                reflectance = fv4(reflectance[0]);
                eta = fv4(eta[0]); k = fv4(k[0]);
                emission = fv4(emission[0]);
            }

            return Interaction(
                bsdf::Bsdf_Union(bsdf::Physical_Bsdf(
                    reflectance, ctx.inside
                    ? (math::max(eta) < math::epsilon ? 0.f : 1.f / eta)
                    : eta, k, alpha_u, alpha_v,
                )),
                emission,
                normal.xyz * 2.f - 1.f,
                degraded,
            );
        }

        public func flags() -> Flags {
            return !emission.empty() ? Flags::emissive : Flags(0);
        }

        func guarded_sample(
            texture::Spectrum_Texture_Tag tex, fv4 fallback,
            texture::Coordinate coord, fv4 lambda
        ) -> fv4 {
            if (tex.empty()) return fallback;
            return tex(coord, lambda);
        }

        func guarded_sample(
            texture::Vector_Texture_Tag tex, fv4 fallback,
            texture::Coordinate coord
        ) -> fv4 {
            if (tex.empty()) return fallback;
            return tex(coord);
        }
    }
}
