#include <metatron/render/light/environment.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/spectra/rgb.hpp>

namespace metatron::light {
		Environment_Light::Environment_Light(
			std::unique_ptr<image::Image> env_map
		): env_map(std::move(env_map)) {}

		auto Environment_Light::operator()(math::Ray const& r) const -> std::optional<Spectrum> {
			auto s_coord = math::cartesion_to_sphere(r.d);
			auto x = s_coord[1] / (2.f * math::pi) * env_map->size[0];
			auto y = s_coord[0] / math::pi * env_map->size[1];
			return std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 4>{(*env_map)[x, y]});
		}

		auto Environment_Light::sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction> {
			auto wi = math::sphere_to_cartesion(u);
			auto x = u[1] / (2.f * math::pi) * env_map->size[0];
			auto y = u[0] / math::pi * env_map->size[1];
			return Interaction{
				std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 4>{(*env_map)[x, y]}),
				wi,
				1.f / (4.f * math::pi)
			};
		}
}
