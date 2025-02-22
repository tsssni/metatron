#include <metatron/core/math/math.hpp>
#include <cstdio>

using namespace metatron;

auto main() -> int {
	auto m0 = math::Matrix<f32, 4, 4>{1.f, 2.f, 3.f, 4.f};
	auto m1 = math::Matrix<f32, 4, 4>{1.f, 2.f, 3.f, 4.f};
	auto m2 = math::Matrix<f32, 4, 4, 4>{1.f};
	auto m = m0 * m1;
	m = m / 2.f;

	for (auto i = 0; i < 4; i++) {
		for (auto j = 0; j < 4; j++) {
			std::printf("%f ", m[i][j]);
		}
		std::printf("\n");
	}
	std::printf("%d", i32(m0 == m1));
	return 0;
}
