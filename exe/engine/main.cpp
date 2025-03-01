#include <metatron/asset/image.hpp>

using namespace metatron;

auto main() -> int {
	auto img = asset::Image::from_path("build/test.png");
	img->to_path("build/test.exr");
	return 0;
}
