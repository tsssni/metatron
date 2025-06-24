#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/resource/media/medium.hpp>
#include <memory>
#include <unordered_map>

namespace metatron::material {
	struct Interaction final {
		std::unique_ptr<bsdf::Bsdf> bsdf;
		spectra::Stochastic_Spectrum emission;
		math::Vector<f32, 3> normal{0.f};
		bool degraded{false};
	};

	struct Material final {
		bsdf::Bsdf const* bsdf;
		
		std::unordered_map<
			std::string,
			texture::Texture<spectra::Stochastic_Spectrum> const*
		> spectrum_textures;

		std::unordered_map<
			std::string,
			texture::Texture<math::Vector<f32, 4>> const*
		> vector_textures;

		auto sample(
			eval::Context const& ctx,
			texture::Coordinate const& coord
		) const -> std::optional<Interaction>;
	};
}
