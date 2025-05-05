#pragma once
#include <metatron/geometry/shape/shape.hpp>
#include <metatron/geometry/material/texture/texture.hpp>
#include <metatron/geometry/material/bsdf/bsdf.hpp>
#include <memory>

namespace metatron::material {
	struct Interaction final {
		std::unique_ptr<Bsdf> bsdf;
		spectra::Stochastic_Spectrum L_e;
	};

	struct Material {
		auto virtual sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const -> std::optional<Interaction> = 0;
	};
}
