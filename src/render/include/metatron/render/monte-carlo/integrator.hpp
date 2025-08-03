#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/resource/photo/camera.hpp>
#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/core/math/sampler/sampler.hpp>

namespace mtt::monte_carlo {
	struct Status final {
		math::Ray_Differential ray_differential;
		math::Ray_Differential default_differential;
		math::Transform const* local_to_world;
		math::Transform const* world_to_render;
		math::Transform const* render_to_camera;
		view<media::Medium> medium;
		math::Vector<usize, 2> pixel;
		usize sample_index;
		usize max_depth;
	};

	MTT_POLY_METHOD(integrator_sample, sample);

	struct Integrator final: pro::facade_builder
	::add_convention<integrator_sample, auto (
		Status initial_status,
		view<accel::Acceleration> accel,
		view<emitter::Emitter> emitter,
		view<math::Sampler> sampler
	) const noexcept -> std::optional<spectra::Stochastic_Spectrum>>
	::support<pro::skills::as_view>
	::build {};
}
