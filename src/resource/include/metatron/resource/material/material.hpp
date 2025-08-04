#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/resource/media/medium.hpp>
#include <unordered_map>

namespace mtt::material {
	struct Interaction final {
		poly<bsdf::Bsdf> bsdf;
		spectra::Stochastic_Spectrum emission;
		math::Vector<f32, 3> normal{0.f};
		bool degraded{false};
	};

	struct Material final {
		poly<bsdf::Bsdf> bsdf;
		
		std::unordered_map<
			std::string,
			view<texture::Texture<spectra::Stochastic_Spectrum>>
		> spectrum_textures;

		std::unordered_map<
			std::string,
			view<texture::Texture<math::Vector<f32, 4>>>
		> vector_textures;

		auto sample(
			eval::Context const& ctx,
			texture::Coordinate const& coord
		) const noexcept -> std::optional<Interaction>;
	};
}
