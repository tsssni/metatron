#include <metatron/asset/image.hpp>

using namespace metatron;

auto main() -> int {
	auto img = asset::Image({1024, 1024, 4, sizeof(f32)});
	img[512, 512] = {1.f, 2.f, 3.f, 4.f};
	auto pixel = math::Vector<f32, 4>{img[512, 512]};
	if (pixel == math::Vector<f32, 4>{1.f, 2.f, 3.f, 4.f}) return 0;
	return 1;
}
