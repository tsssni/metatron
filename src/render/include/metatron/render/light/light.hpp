#pragma once
#include <metatron/core/math/ray.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/eval/context.hpp>

namespace metatron::light {
	struct Interaction final {
		spectra::Stochastic_Spectrum Le;
		math::Vector<f32, 3> wi;
		math::Vector<f32, 3> p;
		f32 t;
		f32 pdf;
	};

	struct Light {
		auto virtual operator()(
			eval::Context const& ctx
		) const -> std::optional<Interaction> = 0;
		auto virtual sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const -> std::optional<Interaction> = 0;
	};
}
