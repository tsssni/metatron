#pragma once

namespace metatron::math {
	struct Exponential_Distribution final {
		Exponential_Distribution(f32 a);
		auto operator()(f32 x) -> f32;
		auto sample(f32 u) -> f32;

	private:
		f32 a;
	};
}
