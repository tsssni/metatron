#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <vector>
#include <string_view>

namespace mtt::image {
	struct Image final {
		struct Pixel final {
			Pixel(Image const* image, byte* start) noexcept;
			explicit operator math::Vector<f32, 4>() const noexcept;
			auto operator=(math::Vector<f32, 4> const& v) noexcept -> void;
			auto operator+=(math::Vector<f32, 4> const& v) noexcept -> void;
		private:
			Image const* image;
			byte* start;
		};

		// 0: width, 1: height, 2: channels, 3: stride
		union {
			struct {
				usize width;
				usize height;
				usize channels;
				usize stride;
			};
			math::Vector<usize, 4> size;
		} const;
		color::Color_Space const* color_space;
		bool linear;

		Image(
			math::Vector<usize, 4> const& size,
			color::Color_Space const* color_space,
			bool linear = false
		) noexcept;

		auto operator[](usize x, usize y) noexcept -> Pixel;
		auto operator[](usize x, usize y) const noexcept -> Pixel const;

		auto static from_path(std::string_view path, bool linear = false) noexcept -> poly<Image>;
		auto to_path(std::string_view path) const noexcept -> void;

	private:
		friend Pixel;
		std::vector<byte> pixels;
	};
}
