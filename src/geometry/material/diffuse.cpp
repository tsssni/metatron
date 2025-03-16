#include <metatron/geometry/material/diffuse.hpp>

namespace metatron::material {
	Diffuse_Material::Diffuse_Material(std::unique_ptr<Spectrum_Texture> R, std::unique_ptr<Spectrum_Texture> T)
		: R(std::move(R)), T(std::move(T)) {}

	auto Diffuse_Material::sample(Context const& ctx) const -> std::optional<std::unique_ptr<Bsdf>> {
		auto R_spec = R->sample(ctx);
		auto T_spec = T->sample(ctx);
		return std::make_unique<Lambertian_Bsdf>(std::move(R_spec), std::move(T_spec));
	}
}
