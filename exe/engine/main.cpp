#include <metatron/core/math/types.hpp>
#include <cstdio>

using namespace metatron;

auto main() -> int {
	math::Matrix4 m = {1.f};
	for (auto i = 0; i < 4; i++) {
		for (auto j = 0; j < 4; j++) {
			std::printf("%f ", m[i][j]);
		}
		std::printf("\n");
	}
	return 0;
}
