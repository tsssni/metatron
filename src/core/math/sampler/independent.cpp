#include <metatron/core/math/sampler/independent.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::math {
	Independent_Sampler::Independent_Sampler(usize seed)
		: rng(seed), dist(1e-4, 1.f - 1e-4) {}

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
