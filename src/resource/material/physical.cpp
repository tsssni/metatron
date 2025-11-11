#include <metatron/resource/material/physical.hpp>
#include <metatron/resource/bsdf/physical.hpp>

namespace mtt::material {
    auto Physical_Material::sample (
        eval::Context const& ctx,
        image::Coordinate const& coord
    ) const noexcept -> std::optional<Interaction> {
        auto guarded_sample = [&]<typename T>(T tex, auto const& fallback) {
            if (!tex) return fallback;
            if constexpr (std::is_same_v<T, proxy<texture::Spectrum_Texture>>)
                return (*tex.data())(coord, ctx.spec);
            else return (*tex.data())(coord);
        };

        auto invalid_spec = spectra::Stochastic_Spectrum{};
        auto zero_spec = ctx.spec & spectra::Spectrum::spectra["zero"];
        auto reflectance = guarded_sample(this->reflectance, invalid_spec);
        auto eta = guarded_sample(this->eta, invalid_spec);
        auto k = guarded_sample(this->k, invalid_spec);
        auto emission = guarded_sample(this->emission, zero_spec);

        auto alpha_u = 0.001f;
        auto alpha_v = 0.001f;
        if (this->alpha) {
            alpha_u = alpha_v = (*this->alpha.data())(coord)[0];
        } else {
            alpha_u = guarded_sample(this->alpha_u, math::Vector<f32, 4>{alpha_u})[0];
            alpha_v = guarded_sample(this->alpha_v, math::Vector<f32, 4>{alpha_v})[0];
        }
        auto normal = guarded_sample(this->normal, math::Vector<f32, 4>{0.5f, 0.5f, 1.f, 0.f});

        auto bsdf = make_poly<bsdf::Bsdf, bsdf::Physical_Bsdf>(
            ctx.spec, reflectance, eta, k,
            alpha_u, alpha_v, ctx.inside
        );
        auto degraded = bsdf->degrade();

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
