#pragma once
#include <metatron/core/image/image.hpp>

namespace metatron::image {
	struct Stb_Image final {
		auto static from_path(std::string_view path) -> std::unique_ptr<Image>;
	};

	struct Exr_Image final {
		auto static from_path(std::string_view path) -> std::unique_ptr<Image>;
		auto static to_path(std::string_view path, Image const& image) -> void;
	};
}
