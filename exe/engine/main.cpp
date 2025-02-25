#include <metatron/asset/image.hpp>

using namespace metatron;

auto main() -> int {
	auto img = asset::Image({1024, 1024, 4, sizeof(f32)});
	for (auto j = 0; j < 1024; j++) {
		for (auto i = 0; i < 1024; i++) {
			img[i, j] = {1.f};
		}
	}
	img.to_path("build/test.exr");
	return 0;
}
