#include <metatron/render/light/environment.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/distribution/cosine-hemisphere.hpp>
#include <metatron/core/spectra/rgb.hpp>

namespace metatron::light {
		Environment_Light::Environment_Light(
			std::unique_ptr<material::Texture<spectra::Stochastic_Spectrum>> env_map
		): env_map(std::move(env_map)) {}

		auto Environment_Light::operator()(
			eval::Context const& ctx
		) const -> std::optional<Interaction> {
			auto s = math::cartesion_to_sphere(ctx.r.d);
			auto u = s[1] / (2.f * math::pi);
			auto v = s[0] / math::pi;
			auto spec = env_map->sample(ctx, {{u, v}});
			auto dist = math::Cosine_Hemisphere_Distribution{};
			return Interaction{
				ctx.L & spec,
				{},
				{},
				{},
				ctx.n != math::Vector<f32, 3>{0.f}
				? dist.pdf(math::dot(ctx.n, ctx.r.d))
				: 1.f / (4.f * math::pi)
			};
		}

		auto Environment_Light::sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const -> std::optional<Interaction> {
			auto dist = math::Cosine_Hemisphere_Distribution{};
			auto wi = dist.sample(u);
			auto n = math::Vector<f32, 3>{0.f};
			if (ctx.n != n) {
				auto local_to_render = math::Quaternion<f32>::from_rotation_between({0.f, 1.f, 0.f}, ctx.n);
				wi = math::Vector<f32, 3>{math::rotate(math::Vector<f32, 4>{wi}, local_to_render)};
				n = ctx.n;
			}
			
			auto intr = (*this)({{}, n, {{}, wi}, ctx.L}).value();
			intr.wi = wi;
			intr.p = ctx.p + 65536.f * wi;
			intr.t = 65536.f;
			return intr;
		}
}
