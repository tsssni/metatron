#include <metatron/resource/light/parallel.hpp>

namespace mtt::light {
    Parallel_Light::Parallel_Light(view<spectra::Spectrum> L) noexcept: L(L) {}

    auto Parallel_Light::operator()(
        math::Ray const& r,
        spectra::Stochastic_Spectrum const& spec
    ) const noexcept -> std::optional<Interaction> {
        return {};
    }

    auto Parallel_Light::sample(
        eval::Context const& ctx,
        math::Vector<f32, 2> const& u
    ) const noexcept -> std::optional<Interaction> {
        auto constexpr wi = math::Vector<f32, 3>{0.f, 0.f, -1.f};
        return Interaction{
            .L = ctx.spec & L,
            .wi = wi,
            .p = ctx.r.o + 65504.f * wi,
            .t = 65504.f,
        };
    }

    auto Parallel_Light::pdf(
        math::Ray const& r,
        math::Vector<f32, 3> const& np
    ) const noexcept -> f32 {
        return 1.f;
    }

    auto Parallel_Light::flags() const noexcept -> Flags {
        return delta;
    }
}
