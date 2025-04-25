#pragma once
#include <metatron/geometry/material/bsdf/bsdf.hpp>

namespace metatron::material {
	struct Cross_Bsdf final: Bsdf {
		Cross_Bsdf();
		auto operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi,
			spectra::Stochastic_Spectrum const& L
		) const -> std::optional<bsdf::Interaction>;
		auto sample(eval::Context const& ctx, math::Vector<f32, 3> const& u) const -> std::optional<bsdf::Interaction>;


	};
}
