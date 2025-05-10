#pragma once
#include <metatron/render/light/light.hpp>
#include <metatron/geometry/material/texture/texture.hpp>
#include <metatron/core/image/image.hpp>
#include <metatron/core/math/distribution/cosine-hemisphere.hpp>
#include <metatron/core/math/distribution/sphere.hpp>

namespace metatron::light {
	struct Environment_Light final: Light {
		Environment_Light(std::unique_ptr<material::Texture<spectra::Stochastic_Spectrum>> env_map);

		auto operator()(
			eval::Context const& ctx
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const -> std::optional<Interaction>;

	private:
		std::unique_ptr<material::Texture<spectra::Stochastic_Spectrum>> env_map;
		math::Cosine_Hemisphere_Distribution surface_distr;
		math::Sphere_Distribution volume_distr;
	};
}
