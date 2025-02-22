#include "metatron/core/math/matrix/types.hpp"
#include <metatron/core/math/math.hpp>
#include <cstdio>

using namespace metatron;

auto main() -> int {
	auto m0 = math::Matrix<f32, 4, 4>{};
	auto m1 = math::Matrix<f32, 3, 3>{1.f};
	m0 = m1;
	for (auto i = 0; i < 4; i++) {
		for (auto j = 0; j < 4; j++) {
			std::printf("%f ", m0[i][j]);
		}
		std::printf("\n");
	}
	return 0;
}
