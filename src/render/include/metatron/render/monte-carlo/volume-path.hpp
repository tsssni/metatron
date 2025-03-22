#pragma once
#include <metatron/render/monte-carlo/integrator.hpp>

namespace metatron::mc {
	struct Volume_Path_Integrator final: Integrator {
		auto sample(
			Context ctx,
			accel::Acceleration const& accel,
			emitter::Emitter const& emitter,
			math::Sampler const& sampler
		) const -> std::optional<spectra::Stochastic_Spectrum>;
	};
}
