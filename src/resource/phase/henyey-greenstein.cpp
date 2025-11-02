#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/quaternion.hpp>

namespace mtt::phase {
    auto Henyey_Greenstein_Phase_Function::operator()(
        math::Vector<f32, 3> const& wo,
        math::Vector<f32, 3> const& wi
    ) const noexcept -> std::optional<Interaction> {
        auto f = math::guarded_div((1.f - g * g) / (4.f * math::pi), std::pow(1.f + g * g + 2.f * g * math::dot(-wo, wi), 1.5f));
        auto spec_f = spectra::Constant_Spectrum{f};
        return Interaction{
            spectrum & (&spec_f),
            wi,
            f
        };
    }

    auto Henyey_Greenstein_Phase_Function::sample(eval::Context const& ctx, math::Vector<f32, 2> const& u)
        const noexcept -> std::optional<Interaction> {
        auto cos_theta = math::abs(g) < math::epsilon<f32>
            ? 1.f - 2.f * u[0]
            : -1.f / (2.f * g) * (1.f + g * g - std::pow(math::guarded_div(1.f - g * g, 1.f + g - 2.f * g * u[0]), 2.f));
        auto phi = 2.f * math::pi * u[1];

        auto wi = math::unit_spherical_to_cartesian(cos_theta, phi);
        auto x = (*this)({0.f, -1.f, 0.f}, wi);
        return x;
    }
}
