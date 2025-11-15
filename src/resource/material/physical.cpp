#include <metatron/resource/material/physical.hpp>
#include <metatron/resource/bsdf/physical.hpp>

namespace mtt::material {
    auto Physical_Material::sample(
        cref<eval::Context> ctx,
        cref<image::Coordinate> coord
    ) const noexcept -> opt<Interaction> {
        auto guarded_sample = [&]<typename T>(T tex, auto const& fallback) {
            if (!tex) return fallback;
            if constexpr (std::is_same_v<T, tag<texture::Spectrum_Texture>>)
                return (*tex.data())(coord, ctx.lambda);
            else return (*tex.data())(coord);
        };
        auto reflectance = guarded_sample(this->reflectance, fv4{0.f});
        auto eta = guarded_sample(this->eta, fv4{0.f});
        auto k = guarded_sample(this->k, fv4{0.f});
        auto emission = guarded_sample(this->emission, fv4{0.f});

        auto alpha_u = 0.001f;
        auto alpha_v = 0.001f;
        if (this->alpha) {
            alpha_u = alpha_v = (*this->alpha.data())(coord)[0];
        } else {
            alpha_u = guarded_sample(this->alpha_u, fv4{alpha_u})[0];
            alpha_v = guarded_sample(this->alpha_v, fv4{alpha_v})[0];
        }
        auto normal = guarded_sample(this->normal, fv4{0.5f, 0.5f, 1.f, 0.f});

        auto lambda = ctx.lambda;
        auto degraded = true
        && eta != fv4{0.f} && reflectance == fv4{0.f} && k == fv4{0.f}
        && !math::constant(eta);
        if (degraded) {
            lambda = fv4{lambda[0]};
            reflectance = fv4{reflectance[0]};
            eta = fv4{eta[0]}; k = fv4{k[0]};
            emission = fv4{emission[0]};
        }
        auto bsdf = make_obj<bsdf::Bsdf, bsdf::Physical_Bsdf>(
            reflectance, eta, k,
            alpha_u, alpha_v, ctx.inside
        );

        return Interaction{
            .bsdf = std::move(bsdf),
            .emission = emission,
            .normal = math::shrink(normal) * 2.f - 1.f,
            .degraded = degraded,
        };
    }

    auto Physical_Material::flags() const noexcept -> Flags {
        return emission ? Flags::emissive : Flags(0);
    }
}
