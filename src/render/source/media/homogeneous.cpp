#include <metatron/render/media/homogeneous.hpp>
#include <metatron/core/distribution/exponential.hpp>

namespace mtt::render {

auto Homogeneous_Medium::Iterator::march(f32 u) noexcept -> Medium_Interaction<Henyey_Greenstein_Phase_Function> {
    auto sigma_a = (lambda & medium->sigma_a);
    auto sigma_s = (lambda & medium->sigma_s);
    auto sigma_t = sigma_a + sigma_s;
    auto sigma_maj = sigma_t;
    auto sigma_n = fv4{0.f};

    auto distr = core::Exponential_Distribution{sigma_t[0]};
    auto t_u = distr.sample(u);
    auto t = core::min(t_u, t_max);
    auto pdf = t < t_max ? distr.pdf(t) : distr.pdf(t) / sigma_t[0];
    auto sigma_e = (lambda & medium->sigma_e);
    auto transmittance = core::exp(-sigma_maj * t);

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

auto Homogeneous_Medium::begin(core::Ray_Context const& ctx, f32 t_max) const noexcept -> Iterator {
    return {this, ctx.r, ctx.lambda, t_max};
}

}
