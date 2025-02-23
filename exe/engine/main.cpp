#include "metatron/core/math/matrix.hpp"
#include <metatron/core/math/math.hpp>
#include <cstdio>

using namespace metatron;

auto main() -> int {
	auto m0 = math::Matrix<f32, 4, 4>{
		{2.f, 1.f, 0.f, 0.f},
		{0.f, 4.f, 6.f, 0.f},
		{0.f, 0.f, 0.f, 1.f},
		{1.f, 0.f, 1.f, 0.f}
	};
	auto m1 = math::Matrix<f32, 4, 4>{1.f, 2.f, 3.f, 4.f};
	auto m = metatron::math::inverse(m0);

	for (auto i = 0; i < 4; i++) {
		for (auto j = 0; j < 4; j++) {
			std::printf("%f ", m[i][j]);
		}
		std::printf("\n");
	}
	return 0;
}
