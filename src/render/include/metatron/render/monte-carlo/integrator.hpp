#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/volume/media/medium.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/math/sampler/sampler.hpp>

namespace metatron::mc {
	struct Ray final {
		math::Ray r;
		math::Ray rx;
		math::Ray ry;
		media::Medium const* medium;
		hierarchy::Transform const* medium_transform;
	};

	struct Integrator {
		auto virtual sample(
			Ray ray,
			accel::Acceleration const& accel,
			emitter::Emitter const& emitter,
			math::Sampler const& sampler
		) const -> std::optional<spectra::Stochastic_Spectrum> = 0;
	};
}
