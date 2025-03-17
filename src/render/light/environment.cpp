#include <metatron/render/light/environment.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/spectra/rgb.hpp>

namespace metatron::light {
		Environment_Light::Environment_Light(
			std::unique_ptr<image::Image> env_map
		): env_map(std::move(env_map)) {}

		auto Environment_Light::operator()(math::Ray const& r, spectra::Stochastic_Spectrum const& L) const -> std::optional<spectra::Stochastic_Spectrum> {
			return (*this)(math::cartesion_to_sphere(r.d), L);
		}

		auto Environment_Light::sample(eval::Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction> {
			auto wi = math::sphere_to_cartesion(u);
			return Interaction{
				(*this)(u, *ctx.L).value(),
				wi,
				*ctx.p + 65536.f * wi,
				1.f / (4.f * math::pi)
			};
		}

		auto Environment_Light::operator()(math::Vector<f32, 2> const& s, spectra::Stochastic_Spectrum const& L) const -> std::optional<spectra::Stochastic_Spectrum> {
			auto x = s[1] / (2.f * math::pi) * env_map->size[0];
			auto y = s[0] / math::pi * env_map->size[1];
			auto spec = spectra::Rgb_Spectrum{math::Vector<f32, 4>{(*env_map)[x, y]}};
			return L & spec;
		}
}
