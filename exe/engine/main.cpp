#include <metatron/asset/image/image.hpp>

using namespace metatron;

auto main() -> int {
	auto img = asset::Image::from_path("build/test.exr");
	img->to_path("build/test1.exr");
	return 0;
}
