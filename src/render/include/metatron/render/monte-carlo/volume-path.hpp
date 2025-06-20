#pragma once
#include <metatron/render/monte-carlo/integrator.hpp>

namespace metatron::monte_carlo {
	struct Volume_Path_Integrator final: Integrator {
		auto sample(
			Status initial_status,
			accel::Acceleration const& accel,
			emitter::Emitter const& emitter,
			math::Sampler const& sampler
		) const -> std::optional<spectra::Stochastic_Spectrum>;
	};
}
