#include <metatron/resource/media/homogeneous.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/core/math/distribution/exponential.hpp>

namespace mtt::media {
    auto Homogeneous_Medium::sample(
        cref<math::Context> ctx, f32 t_max, f32 u
    ) const noexcept -> opt<Interaction> {
        auto sigma_a = (ctx.lambda & this->sigma_a);
        auto sigma_s = (ctx.lambda & this->sigma_s);
        auto sigma_t = sigma_a + sigma_s;
        auto sigma_maj = sigma_t;
        auto sigma_n = fv4{0.f};

        auto distr = math::Exponential_Distribution{sigma_t[0]};
        auto t_u = distr.sample(u);
        auto t = math::min(t_u, t_max);
        auto pdf = t < t_max ? distr.pdf(t) : distr.pdf(t) / sigma_t[0];

        auto sigma_e = (ctx.lambda & this->sigma_e);
        auto transmittance = math::foreach([&](f32 value, usize i) {
            return std::exp(-value * t);
        }, sigma_maj);

        return Interaction{
            ctx.r.o + ctx.r.d * t,
            this->phase.to_phase(),
            t,
            transmittance,
            sigma_a,
            sigma_s,
            sigma_n,
            sigma_maj,
            sigma_e
        };
    }
}
