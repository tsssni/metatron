#pragma once
#include <metatron/render/light/emitter.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/geometry/divider/accel.hpp>
#include <metatron/volume/media/medium.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/math/sampler/sampler.hpp>

namespace metatron::mc {
	struct Ray final {
		math::Ray r;
		math::Ray rx;
		math::Ray ry;
		media::Medium const* medium;
	};

	struct Integrator {
		auto virtual sample(
			Ray ray,
			divider::Acceleration const& accel,
			light::Emitter const& emitter,
			math::Sampler const& sampler
		) const -> std::optional<std::unique_ptr<spectra::Stochastic_Spectrum>> = 0;
	};
}
