#include <metatron/core/math/sampler/independent.hpp>

namespace metatron::math {
	Independent_Sampler::Independent_Sampler(usize seed)
		: rng(seed), dist(0.f, 1.f) {}

	auto Independent_Sampler::start(math::Vector<usize, 2> const& pixel, usize idx) -> void {
		this->pixel = pixel;
		this->idx = idx;
	}

	auto Independent_Sampler::generate_1d() const -> f32 {
		return dist(rng);
	}

	auto Independent_Sampler::generate_2d() const -> math::Vector<f32, 2> {
		return {generate_1d(), generate_1d()};
	}

	auto Independent_Sampler::generate_pixel_2d() const -> math::Vector<f32, 2> {
		return generate_2d();
	}
}
