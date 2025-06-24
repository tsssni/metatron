#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/resource/media/medium.hpp>
#include <memory>

namespace metatron::material {
	struct Interaction final {
		std::unique_ptr<bsdf::Bsdf> bsdf;
		spectra::Stochastic_Spectrum emission;
		math::Vector<f32, 3> normal{0.f};
		bool degraded{false};
	};

	struct Material final {
		bsdf::Bsdf const* bsdf;
		media::Medium const* interior_medium;
		media::Medium const* exterior_medium;
		spectra::Spectrum const* eta;
		spectra::Spectrum const* k;
		texture::Texture<spectra::Stochastic_Spectrum> const* reflectance;
		texture::Texture<spectra::Stochastic_Spectrum> const* transmittance;
		texture::Texture<spectra::Stochastic_Spectrum> const* emission;
		texture::Texture<math::Vector<f32, 4>> const* u_roughness;
		texture::Texture<math::Vector<f32, 4>> const* v_roughness;
		texture::Texture<math::Vector<f32, 4>> const* nomral;
		
		auto sample(
			eval::Context const& ctx,
			texture::Coordinate const& coord
		) const -> std::optional<Interaction>;
	};
}
