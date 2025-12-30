#include <metatron/resource/material/physical.hpp>
#include <metatron/resource/bsdf/physical.hpp>

namespace mtt::material {
    auto Physical_Material::sample(
        cref<math::Context> ctx,
        cref<muldim::Coordinate> coord
    ) const noexcept -> opt<Interaction> {
        auto guarded_sample = [&]<typename T>(T tex, auto const& fallback) {
            if (!tex) return fallback;
            if constexpr (std::is_same_v<T, tag<texture::Spectrum_Texture>>)
                return (*tex.data())(coord, ctx.lambda);
            else return (*tex.data())(coord);
        };
        auto reflectance = guarded_sample(this->reflectance, fv4{2.f});
        auto eta = guarded_sample(this->eta, fv4{0.f});
        auto k = guarded_sample(this->k, fv4{0.f});
        auto emission = guarded_sample(this->emission, fv4{0.f});

        auto alpha_u = 0.f;
        auto alpha_v = 0.f;
        if (this->alpha) alpha_u = alpha_v = (*this->alpha.data())(coord)[0];
        if (this->alpha_u) alpha_u = guarded_sample(this->alpha_u, fv4{alpha_u})[0];
        if (this->alpha_v) alpha_v = guarded_sample(this->alpha_v, fv4{alpha_v})[0];
        auto normal = guarded_sample(this->normal, fv4{0.5f, 0.5f, 1.f, 0.f});

        auto lambda = ctx.lambda;
        auto degraded = true
        && reflectance != math::saturate(reflectance)
        && eta != fv4{0.f} && !math::constant(eta)
        && k == fv4{0.f};
        
        if (degraded) {
            lambda = fv4{lambda[0]};
            reflectance = fv4{reflectance[0]};
            eta = fv4{eta[0]}; k = fv4{k[0]};
            emission = fv4{emission[0]};
        }

        return Interaction{
            .bsdf = make_obj<bsdf::Bsdf, bsdf::Physical_Bsdf>(
                reflectance,
                ctx.inside ? 1.f / eta : eta,
                k, alpha_u, alpha_v
            ),
            .emission = emission,
            .normal = math::shrink(normal) * 2.f - 1.f,
            .degraded = degraded,
        };
    }

    auto Physical_Material::flags() const noexcept -> Flags {
        return emission ? Flags::emissive : Flags(0);
    }
}
