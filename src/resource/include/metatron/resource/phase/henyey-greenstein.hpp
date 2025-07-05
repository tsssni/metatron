#pragma once
#include <metatron/resource/phase/phase-function.hpp>

namespace mtt::phase {
	struct Henyey_Greenstein_Phase_Function final: Phase_Function {
		Henyey_Greenstein_Phase_Function(f32 g);
		auto operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi
		) const -> std::optional<Interaction>;
		auto sample(eval::Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction>;
		auto clone(Attribute const& attr) const -> std::unique_ptr<Phase_Function>;
	
	private:
		spectra::Stochastic_Spectrum spectrum;
		f32 g;
	};
}
