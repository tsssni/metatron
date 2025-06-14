#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/resource/media/medium.hpp>
#include <memory>

namespace metatron::material {
	struct Interaction final {
		std::unique_ptr<bsdf::Bsdf> bsdf;
		spectra::Stochastic_Spectrum emission;
		math::Vector<f32, 3> normal;
	};

	struct Material final {
		bsdf::Bsdf const* bsdf;
		media::Medium const* medium;
		texture::Texture<spectra::Stochastic_Spectrum> const* reflectance;
		texture::Texture<spectra::Stochastic_Spectrum> const* transmittance;
		texture::Texture<spectra::Stochastic_Spectrum> const* emission;
		texture::Texture<math::Vector<f32, 4>> const* nomral;
		texture::Texture<math::Vector<f32, 4>> const* mrsa;
		
		auto sample(
			eval::Context const& ctx,
			texture::Coordinate const& coord
		) const -> std::optional<Interaction>;
	};
}
