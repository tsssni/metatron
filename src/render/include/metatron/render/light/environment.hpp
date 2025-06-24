#pragma once
#include <metatron/render/light/light.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/image/image.hpp>
#include <metatron/core/math/distribution/sphere.hpp>
#include <metatron/core/math/distribution/sphere.hpp>

namespace metatron::light {
	struct Environment_Light final: Light {
		Environment_Light(texture::Texture<spectra::Stochastic_Spectrum> const* env_map);

		auto operator()(
			eval::Context const& ctx
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const -> std::optional<Interaction>;
		auto flags() const -> Flags;

	private:
		texture::Texture<spectra::Stochastic_Spectrum> const* env_map;
		math::Cosine_Hemisphere_Distribution surface_distr;
		math::Sphere_Distribution volume_distr;
	};
}
