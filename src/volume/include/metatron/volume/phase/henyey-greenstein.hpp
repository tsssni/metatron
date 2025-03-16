#pragma once
#include <metatron/volume/phase/phase-function.hpp>

namespace metatron::phase {
	struct Henyey_Greenstein_Phase_Function final: Phase_Function {
		Henyey_Greenstein_Phase_Function(f32 g);
		auto operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi
		) const -> f32;
		auto sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction>;
	
	private:
		f32 g;
	};
}
