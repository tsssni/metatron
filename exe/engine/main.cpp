#include "metatron/core/math/matrix/types.hpp"
#include <metatron/core/math/math.hpp>
#include <cstdio>

using namespace metatron;

auto main() -> int {
	auto m0 = math::Matrix<f32, 3, 4>{1.f, 2.f, 3.f};
	auto m1 = math::Matrix<f32, 4, 3>{
		{1.f, 2.f, 3.f},
		{2.f, 3.f, 4.f},
		{3.f, 4.f, 5.f},
		{4.f, 5.f, 6.f}
	};
	auto m = m1 * m0;

	for (auto i = 0; i < 3; i++) {
		for (auto j = 0; j < 4; j++) {
			std::printf("%f ", m0[i][j]);
		}
		std::printf("\n");
	}
	return 0;
}
