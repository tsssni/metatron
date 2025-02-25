#include <metatron/core/math/vector.hpp>
#include <vector>

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

		// 0: width, 1: height, 2: channel, 3: stride
		math::Vector<usize, 4> const size;
		Image(math::Vector<usize, 4> const& size);

		auto operator[](usize x, usize y) -> Pixel;
		auto operator[](usize x, usize y) const -> Pixel const;

	private:
		std::vector<byte> pixels;
	};
}
