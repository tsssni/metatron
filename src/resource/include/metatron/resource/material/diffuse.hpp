#pragma once
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/bsdf/lambertian.hpp>
#include <metatron/resource/texture/texture.hpp>

namespace metatron::material {
	struct Diffuse_Material final: Material {
		Diffuse_Material(
			std::unique_ptr<texture::Texture<spectra::Stochastic_Spectrum>> R,
			std::unique_ptr<texture::Texture<spectra::Stochastic_Spectrum>> T,
			std::unique_ptr<texture::Texture<spectra::Stochastic_Spectrum>> L
		);

		auto sample(
			eval::Context const& ctx,
			texture::Coordinate const& coord
		) const -> std::optional<Interaction>;

	private:
		std::unique_ptr<texture::Texture<spectra::Stochastic_Spectrum>> R;
		std::unique_ptr<texture::Texture<spectra::Stochastic_Spectrum>> T;
		std::unique_ptr<texture::Texture<spectra::Stochastic_Spectrum>> L;
	};
}
