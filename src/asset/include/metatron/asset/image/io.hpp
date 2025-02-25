#include <metatron/asset/image/image.hpp>

namespace metatron::asset {
	auto exr_reader(std::string_view path) -> std::unique_ptr<Image>;
	auto exr_writer(std::string_view path, Image const& image) -> void;
}
