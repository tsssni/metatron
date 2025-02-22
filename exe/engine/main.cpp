#include <metatron/core/math/math.hpp>
#include <cstdio>

using namespace metatron;

auto main() -> int {
	math::Matrix<f32, 1, 3, 4> m0{
		{
			{1.f},
			{2.f},
			{3.f}
		}
	};
	math::Matrix<f32, 1, 4, 5> m1{
		{
			{1.f, 2.f, 3.f, 4.f, 5.f}
		}
	};
	auto m2 = m0 * m1;
	for (auto i = 0; i < 3; i++) {
		for (auto j = 0; j < 5; j++) {
			std::printf("%f ", m2[0][i][j]);
		}
		std::printf("\n");
	}
	return 0;
}
