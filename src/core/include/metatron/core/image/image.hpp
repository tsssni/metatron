#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/color/color-space.hpp>
#include <vector>
#include <string_view>
#include <memory>

namespace metatron::image {
	struct Image final {
		struct Pixel final {
			Pixel(Image const* image, byte* start);
			explicit operator math::Vector<f32, 4>() const;
			auto operator=(math::Vector<f32, 4> const& v) -> void;
			auto operator+=(math::Vector<f32, 4> const& v) -> void;
		private:
			Image const* image;
			byte* start;
		};

		// 0: width, 1: height, 2: channels, 3: stride
		math::Vector<usize, 4> const size;
		color::Color_Space const* color_space;
		bool linear;

		Image(
			math::Vector<usize, 4> const& size,
			color::Color_Space const* color_space,
			bool linear = true
		);

		auto operator[](usize x, usize y) -> Pixel;
		auto operator[](usize x, usize y) const -> Pixel const;

		auto static from_path(std::string_view path, bool linear = true) -> std::unique_ptr<Image>;
		auto to_path(std::string_view path) -> void;

	private:
		friend Pixel;
		std::vector<byte> pixels;
	};
}
