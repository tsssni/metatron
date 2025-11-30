#include <metatron/resource/light/environment.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/distribution/sphere.hpp>

namespace mtt::light {
    auto Environment_Light::operator()(
        cref<math::Ray> r, cref<fv4> lambda
    ) const noexcept -> opt<Interaction> {
        auto [radius, theta, phi] = math::cartesian_to_spherical(r.d);
        auto u = 1.f - phi / (2.f * math::pi);
        auto v = theta / math::pi;
        auto t = (*env_map.data())({{u, v}}, lambda);
        auto J = 2.f * math::sqr(math::pi) * std::sin(theta);
        return Interaction{
            .L = t,
            .wi = r.d,
            .p = r.o + r.d * 65504.f,
            .t = 65504.f,
            .pdf = math::guarded_div(env_map->pdf({u, v}), J),
        };
    }

    auto Environment_Light::sample(
        cref<math::Context> ctx,
        cref<fv2> u
    ) const noexcept -> opt<Interaction> {
        auto uv = env_map->sample(ctx, u);
        auto phi = (1.f - uv[0]) * 2.f * math::pi;
        auto theta = uv[1] * math::pi;
        auto wi = math::unit_spherical_to_cartesian({theta, phi});
        return (*this)({ctx.r.o, wi}, ctx.lambda);
    }

    auto Environment_Light::flags() const noexcept -> Flags {
        return Flags::inf;
    }
}
