#include <metatron/resource/light/area.hpp>
#include <metatron/core/stl/optional.hpp>

namespace mtt::light {
    Area_Light::Area_Light(
        view<shape::Shape> shape,
        usize primitive
    ) noexcept: shape(shape), primitive(primitive) {}

    auto Area_Light::operator()(
        math::Ray const& r,
        spectra::Stochastic_Spectrum const& spec
    ) const noexcept -> std::optional<Interaction> {
        return {};
    }

    auto Area_Light::sample(
        eval::Context const& ctx,
        math::Vector<f32, 2> const& u
    ) const noexcept -> std::optional<Interaction> {
        MTT_OPT_OR_RETURN(s_intr, shape->sample(ctx, u, primitive), {});
        auto L = ctx.spec; L = 0.f; // delay fetching L in integrator material interaction
        return Interaction{
            .L = ctx.spec & spectra::Spectrum::spectra["zero"],
            .wi = math::normalize(s_intr.p - ctx.r.o),
            .p = s_intr.p,
            .t = s_intr.t,
        };
    }

    auto Area_Light::pdf(
        math::Ray const& r,
        math::Vector<f32, 3> const& np
    ) const noexcept -> f32 {
        return shape->pdf(r, np, primitive);
    }

    auto Area_Light::flags() const noexcept -> Flags {
        return Flags(0);
    }
}
