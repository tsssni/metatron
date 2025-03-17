#pragma once
#include <metatron/geometry/shape/shape.hpp>
#include <metatron/geometry/material/texture/texture.hpp>
#include <metatron/geometry/material/bsdf/bsdf.hpp>

namespace metatron::material {
	struct Interaction final {
		std::unique_ptr<Bsdf> bsdf;
		std::unique_ptr<spectra::Stochastic_Spectrum> Le;
	};

	struct Material {
		auto virtual sample(eval::Context const& ctx) const -> std::optional<Interaction> = 0;
	};
}
