#pragma once
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/bsdf/lambertian.hpp>
#include <metatron/resource/texture/texture.hpp>

namespace metatron::material {
	struct Diffuse_Material final: Material {
		Diffuse_Material(Texture_Set&& texture_set);

		auto sample(
			eval::Context const& ctx,
			texture::Coordinate const& coord
		) const -> std::optional<Interaction>;

	private:
		Texture_Set texture_set;
	};
}
