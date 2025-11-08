#include <metatron/resource/light/point.hpp>

namespace mtt::light {
    auto Point_Light::operator()(
        math::Ray const& r,
        spectra::Stochastic_Spectrum const& spec
    ) const noexcept -> std::optional<Interaction> {
        return {};
    }

    auto Point_Light::sample(
        eval::Context const& ctx,
        math::Vector<f32, 2> const& u
    ) const noexcept -> std::optional<Interaction> {
        auto wi = math::normalize(-ctx.r.o);
        auto r = math::length(ctx.r.o);
        return Interaction{
            .L = (ctx.spec & L.data()) / (r * r),
            .wi = wi,
            .p = {0.f},
            .t = r,
            .pdf = 1.f,
        };
    }

    auto Point_Light::flags() const noexcept -> Flags {
        return delta;
    }
}
