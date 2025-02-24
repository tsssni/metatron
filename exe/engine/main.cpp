#include <metatron/core/math/math.hpp>
#include <metatron/rendering/camera.hpp>
#include <cstdio>

using namespace metatron;

auto main() -> int {
	auto q0 = math::rotation_axis_angle({0.f, 1.f, 0.f}, 0.f);
	auto q1 = math::rotation_axis_angle({0.f, 1.f, 0.f}, 3.1415926535f);
	auto q3 = q0 * q1;
	for (auto i = 0; i < 4; i++) {
		std::printf("%f ", q3[i]);
	}
	return 0;
}
