#include "metatron/core/math/matrix.hpp"
#include "metatron/core/math/vector.hpp"
#include <metatron/core/math/math.hpp>
#include <cstdio>

using namespace metatron;

auto main() -> int {
	auto v = math::Vector<f32, 3>{1.f, -2.f, 3.f};
	auto n = math::Vector<f32, 3>{0.f, 1.f, 0.f};
	auto r = math::reflect(math::normalize(v), n);
	for (auto i = 0; i < 3; i++) {
		std::printf("%f ", r[i]);
	}
	return 0;
}
