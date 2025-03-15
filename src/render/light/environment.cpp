#include <metatron/render/light/environment.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::light {
		Environment_Light::Environment_Light(std::unique_ptr<material::Spectrum_Image_Texture> texture)
			: texture(std::move(texture)) {}

		auto Environment_Light::operator()(math::Ray const& r) const -> std::optional<Spectrum> {
			auto s_coord = math::cartesion_to_sphere(r.d);
			auto u = s_coord[1] / (2.f * math::pi);
			auto v = s_coord[0] / math::pi;
			return (*texture)({u, v});
		}

		auto Environment_Light::sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction> {
			// TODO: not implemented
			return {};
		}
}
