#pragma once
#include <metatron/geometry/material/material.hpp>
#include <metatron/geometry/material/bsdf/lambertian.hpp>
#include <metatron/geometry/material/texture/spectrum/spectrum.hpp>

namespace metatron::material {
	struct Diffuse_Material final: Material {
		Diffuse_Material(std::unique_ptr<Spectrum_Texture> R, std::unique_ptr<Spectrum_Texture> T);
		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const -> std::optional<Interaction>;

	private:
		std::unique_ptr<Spectrum_Texture> R;
		std::unique_ptr<Spectrum_Texture> T;
	};
}
