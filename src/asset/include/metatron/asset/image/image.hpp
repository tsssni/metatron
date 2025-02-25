#pragma once
#include <metatron/core/math/vector.hpp>
#include <vector>
#include <string_view>
#include <memory>

namespace metatron::asset {
	struct Image final {
		struct Pixel final {
			Pixel(Image const* image, byte* start);
			explicit operator math::Vector<f32, 4>() const;
			auto operator=(math::Vector<f32, 4> const& v) -> void;
		private:
			Image const* image;
			byte* start;
		};

		// 0: width, 1: height, 2: channels, 3: stride
		math::Vector<usize, 4> const size;
		Image(math::Vector<usize, 4> const& size);

		auto operator[](usize x, usize y) -> Pixel;
		auto operator[](usize x, usize y) const -> Pixel const;

		auto static from_path(std::string_view path) -> std::unique_ptr<Image>;
		auto to_path(std::string_view path) -> void;

	private:
		std::vector<byte> pixels;
	};
}
