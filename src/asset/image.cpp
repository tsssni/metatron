#include <metatron/asset/image.hpp>

namespace metatron::asset {
	Image::Pixel::Pixel(Image const* image, byte* start)
	: image(image), start(start) {}

	Image::Pixel::operator math::Vector<f32, 4>() const {
		auto pixel = math::Vector<f32, 4>{};
		for (auto i = 0; i < image->size[2]; i++) {
			switch (image->size[3]) {
				case 1:
					pixel[i] = *(start + i) / 255.f;
					break;
				case 4:
					pixel[i] = *((f32*)(start) + i);
					break;
				default:
					break;
			}
		}
		return pixel;
	}

	auto Image::Pixel::operator=(math::Vector<f32, 4> const& v) -> void{
		for (auto i = 0; i < image->size[2]; i++) {
			auto* pixel = start + image->size[3] * i; 
			switch (image->size[3]) {
				case 1:
					*pixel = byte(v[i] * 255.f);
					break;
				case 4:
					*((f32*)pixel) = v[i];
					break;
				default:
					break;
			}
		}
	}

	Image::Image(math::Vector<usize, 4> const& size) 
	: size(size), pixels(size[0] * size[1] * size[2] * size[3]) {}

	auto Image::operator[](usize x, usize y) -> Pixel {
		auto offset = (y * size[0] + x) * size[2] * size[3];
		return Pixel{this, &pixels[offset]};
	}

	auto Image::operator[](usize x, usize y) const -> Pixel const {
		auto offset = (y * size[0] + x) * size[2] * size[3];
		return (Pixel const){this, const_cast<byte*>(&pixels[offset])};
	}
}
