#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/core/math/sampler/sampler.hpp>

namespace metatron::monte_carlo {
	struct Status final {
		math::Ray_Differential ray_differential;
		math::Ray_Differential default_differential;
		math::Transform const* local_to_world;
		math::Transform const* world_to_render;
		math::Transform const* render_to_camera;
		media::Medium const* medium;
		math::Vector<usize, 2> pixel;
		usize sample_index;
		usize max_depth;
	};

	struct Integrator {
		auto virtual sample(
			Status initial_status,
			accel::Acceleration const& accel,
			emitter::Emitter const& emitter,
			math::Sampler const& sampler
		) const -> std::optional<spectra::Stochastic_Spectrum> = 0;
	};
}
