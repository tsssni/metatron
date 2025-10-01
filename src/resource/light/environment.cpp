#include <metatron/resource/light/environment.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/quaternion.hpp>

namespace mtt::light {
    Environment_Light::Environment_Light(
        view<texture::Spectrum_Texture> env_map
    ) noexcept: env_map(env_map) {}

    auto Environment_Light::operator()(
        eval::Context const& ctx
    ) const noexcept -> std::optional<Interaction> {
        auto s = math::cartesian_to_unit_spherical(ctx.r.d);
        auto u = 1.f - s[1] / (2.f * math::pi);
        auto v = s[0] / math::pi;
        auto spec = env_map->sample(ctx, {{u, v}});
        return Interaction{
            .L = ctx.spec & (&spec),
            .wi = {}, .p = {}, .t = {},
            .pdf = ctx.n != math::Vector<f32, 3>{0.f}
            ? math::Cosine_Hemisphere_Distribution{}.pdf(math::dot(ctx.n, ctx.r.d))
            : math::Sphere_Distribution{}.pdf()
        };
    }

    auto Environment_Light::sample(
        eval::Context const& ctx,
        math::Vector<f32, 2> const& u
    ) const noexcept -> std::optional<Interaction> {
        auto wi = math::Vector<f32, 3>{};
        auto n = math::Vector<f32, 3>{0.f};
        if (ctx.n != n) {
            auto local_to_render = math::Quaternion<f32>::from_rotation_between({0.f, 1.f, 0.f}, ctx.n);
            wi = math::Vector<f32, 3>{math::rotate(math::expand(
                math::Cosine_Hemisphere_Distribution{}.sample(u), 0.f
            ), local_to_render)};
            n = ctx.n;
        } else {
            wi = math::Sphere_Distribution{}.sample(u);
        }
        
        auto intr = (*this)({{ctx.r.o, wi}, n, ctx.spec}).value();
        intr.wi = wi;
        intr.p = ctx.r.o + 65504.f * wi;
        intr.t = 65504.f;
        return intr;
    }

    auto Environment_Light::flags() const noexcept -> Flags {
        return Flags::inf;
    }
}
