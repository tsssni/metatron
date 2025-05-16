#include <metatron/geometry/material/diffuse.hpp>
#include <metatron/core/spectra/constant.hpp>

namespace metatron::material {
	Diffuse_Material::Diffuse_Material(
		std::unique_ptr<material::Texture<spectra::Stochastic_Spectrum>> R,
		std::unique_ptr<material::Texture<spectra::Stochastic_Spectrum>> T,
		std::unique_ptr<material::Texture<spectra::Stochastic_Spectrum>> L
	): R(std::move(R)), T(std::move(T)), L(std::move(L)) {}

	auto Diffuse_Material::sample(
		eval::Context const& ctx,
		Coordinate const& coord
	) const -> std::optional<Interaction> {
		return Interaction{
			std::make_unique<Lambertian_Bsdf>(
				std::make_unique<spectra::Stochastic_Spectrum>(R->sample(ctx, coord)),
				std::make_unique<spectra::Stochastic_Spectrum>(T->sample(ctx, coord))
			),
			L->sample(ctx, coord)
		};
	}
}
