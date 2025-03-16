#pragma once
#include <metatron/render/monte-carlo/integrator.hpp>

namespace metatron::mc {
	struct Volume_Path_Integrator final: Integrator {
		auto sample(
			Ray ray,
			divider::Acceleration const& accel,
			light::Emitter const& emitter,
			math::Sampler const& sampler
		) const -> std::optional<std::unique_ptr<spectra::Stochastic_Spectrum>>;
	};
}
