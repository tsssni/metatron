#include <metatron/core/math/distribution/exponential.hpp>
#include <cmath>

namespace metatron::math {
	Exponential_Distribution::Exponential_Distribution(f32 a): a(a) {}
	auto Exponential_Distribution::operator()(f32 x) -> f32 {
		return a * std::exp(-a * x);
	}

	auto Exponential_Distribution::sample(f32 u) -> f32 {
		return -std::log(1.f - u) / a;
	}
}
