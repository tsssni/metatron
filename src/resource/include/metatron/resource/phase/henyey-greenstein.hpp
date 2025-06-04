#pragma once
#include <metatron/resource/phase/phase-function.hpp>

namespace metatron::phase {
	struct Henyey_Greenstein_Phase_Function final: Phase_Function {
		Henyey_Greenstein_Phase_Function(f32 g);
		auto operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi,
			spectra::Stochastic_Spectrum const& L
		) const -> std::optional<Interaction>;
		auto sample(eval::Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction>;
	
	private:
		f32 g;
	};
}
