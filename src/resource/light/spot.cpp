#include <metatron/resource/light/spot.hpp>

namespace mtt::light {
    Spot_Light::Spot_Light(
        view<spectra::Spectrum> L,
        f32 falloff_start_theta,
        f32 falloff_end_theta
    ) noexcept:
    L(L),
    falloff_start_cos_theta(std::cos(falloff_start_theta)),
    falloff_end_cos_theta(std::cos(falloff_end_theta)) {}

    auto Spot_Light::operator()(
        eval::Context const& ctx
    ) const noexcept -> std::optional<Interaction> {
        return {};
    }

    auto Spot_Light::sample(
        eval::Context const& ctx,
        math::Vector<f32, 2> const& u
    ) const noexcept -> std::optional<Interaction> {
        auto smoothstep = [](f32 start, f32 end, f32 x) -> f32 {
            if (x < start) {
                return 0.f;
            } else if (x > end) {
                return 1.f;
            }
            auto t = (x - start) / (end - start);
            return t * t * (3.f - 2.f * t);
        };

        auto constexpr d = math::Vector<f32, 3>{0.f, 0.f, 1.f};
        auto wi = math::normalize(-ctx.r.o);
        auto r = math::length(ctx.r.o);

        auto cos_theta = math::dot(d, -wi);
        auto intensity = smoothstep(
            falloff_end_cos_theta,
            falloff_start_cos_theta,
            cos_theta
        );

        return Interaction{
            .L = (ctx.spec & L) * intensity / (r * r),
            .wi = wi,
            .p = {0.f},
            .t = r,
            .pdf = 1.f
        };
    }

    auto Spot_Light::flags() const noexcept -> Flags {
        return delta;
    }
}
