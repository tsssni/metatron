#include <metatron/resource/material/diffuse.hpp>
#include <metatron/resource/spectra/constant.hpp>

namespace metatron::material {
	Diffuse_Material::Diffuse_Material(Texture_Set&& texture_set)
	: texture_set(std::move(texture_set)) {}

	auto Diffuse_Material::sample(
		eval::Context const& ctx,
		texture::Coordinate const& coord
	) const -> std::optional<Interaction> {
		return Interaction{
			std::make_unique<bsdf::Lambertian_Bsdf>(
				std::make_unique<spectra::Stochastic_Spectrum>(texture_set.R->sample(ctx, coord)),
				std::make_unique<spectra::Stochastic_Spectrum>(texture_set.T->sample(ctx, coord))
			),
			texture_set.L->sample(ctx, coord),
			texture_set.N->sample(ctx, coord)
		};
	}
}
