#pragma once
#include <metatron/render/monte-carlo/integrator.hpp>

namespace mtt::monte_carlo {
	struct Volume_Path_Integrator final {
		auto sample(
			Status initial_status,
			view<accel::Acceleration> accel,
			view<emitter::Emitter> emitter,
			view<math::Sampler> sampler
		) const noexcept -> std::optional<spectra::Stochastic_Spectrum>;
	};
}
