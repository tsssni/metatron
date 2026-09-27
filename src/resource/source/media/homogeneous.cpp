#include <metatron/resource/media/homogeneous.hpp>
#include <metatron/core/math/distribution/exponential.hpp>

namespace mtt::media {
    auto Homogeneous_Medium::Iterator::march(f32 u) noexcept -> Interaction<phase::Henyey_Greenstein_Phase_Function> {
        auto sigma_a = (lambda & medium->sigma_a);
        auto sigma_s = (lambda & medium->sigma_s);
        auto sigma_t = sigma_a + sigma_s;
        auto sigma_maj = sigma_t;
        auto sigma_n = fv4{0.f};

        auto distr = math::Exponential_Distribution{sigma_t[0]};
        auto t_u = distr.sample(u);
        auto t = math::min(t_u, t_max);
        auto pdf = t < t_max ? distr.pdf(t) : distr.pdf(t) / sigma_t[0];
        auto sigma_e = (lambda & medium->sigma_e);
        auto transmittance = math::exp(-sigma_maj * t);

        return {
            medium->phase,
            r.o + r.d * t,
            t,
            transmittance,
            sigma_a,
            sigma_s,
            sigma_n,
            sigma_maj,
            sigma_e
        };
    }

    auto Homogeneous_Medium::begin(math::Context const& ctx, f32 t_max) const noexcept -> Iterator {
        return {this, ctx.r, ctx.lambda, t_max};
    }
}
