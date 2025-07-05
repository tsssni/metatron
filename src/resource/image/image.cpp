#include <metatron/resource/image/image.hpp>
#include <OpenImageIO/imageio.h>

namespace mtt::image {
	Image::Pixel::Pixel(Image const* image, byte* start)
	: image(image), start(start) {}

	Image::Pixel::operator math::Vector<f32, 4>() const {
		auto pixel = math::Vector<f32, 4>{};
		for (auto i = 0; i < image->size[2]; i++) {
			switch (image->size[3]) {
				case 1:
					pixel[i] = image->linear
						? *(start + i) / 255.f
						: image->color_space->decode(*(start + i) / 255.f);
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

	auto Image::Pixel::operator=(math::Vector<f32, 4> const& v) -> void {
		for (auto i = 0; i < image->size[2]; i++) {
			auto* pixel = start + image->size[3] * i; 
			switch (image->size[3]) {
				case 1:
					*pixel = image->linear
						? byte(v[i] * 255.f)
						: byte(image->color_space->encode(v[i]) * 255.f);
					break;
				case 4:
					*((f32*)pixel) = v[i];
					break;
				default:
					break;
			}
		}
	}

	auto Image::Pixel::operator+=(math::Vector<f32, 4> const& v) -> void {
		*this = math::Vector<f32, 4>(*this) + v;
	}

	Image::Image(
		math::Vector<usize, 4> const& size,
		color::Color_Space const* color_space,
		bool linear
	):
	size(size),
	pixels(size[0] * size[1] * size[2] * size[3]),
	color_space(color_space),
	linear(linear) {}

	auto Image::operator[](usize x, usize y) -> Pixel {
		auto offset = (y * width + x) * channels * stride;
		return Pixel{this, &pixels[offset]};
	}

	auto Image::operator[](usize x, usize y) const -> Pixel const {
		auto offset = (y * width + x) * channels * stride;
		return (Pixel const){this, const_cast<byte*>(&pixels[offset])};
	}

	namespace {
		auto to_color_space(std::string_view cs) -> color::Color_Space const* {
			if (cs == "sRGB") {
				return color::Color_Space::sRGB.get();
			} else {
				// have not supported other color spaces yet
				return color::Color_Space::sRGB.get();
			}
		}

		auto from_color_space(color::Color_Space const* cs) -> std::string_view {
			if (cs == color::Color_Space::sRGB.get()) {
				return "sRGB";
			} else {
				// have not supported other color spaces yet
				return "sRGB";
			}
		}
	}

	auto Image::from_path(std::string_view path, bool linear) -> std::unique_ptr<Image> {
		auto in = OIIO::ImageInput::open(std::string{path});
		if (!in) {
			std::printf("cannot find image %s\n", path.data());
			std::abort();
		}

		auto& spec = in->spec();
		auto cs_name = spec.get_string_attribute("oiio:ColorSpace");
		auto color_space = (color::Color_Space*)nullptr;

		auto image = std::make_unique<Image>(
			math::Vector<usize, 4>{
				usize(spec.width),
				usize(spec.height),
				usize(spec.nchannels),
				spec.format.size()
			},
			to_color_space(spec.get_string_attribute("oiio:ColorSpace")),
			linear
		);

		auto success = in->read_image(0, 0, 0, spec.nchannels, spec.format, image->pixels.data());
		if (!success) {
			std::printf("can not read image %s\n", path.data());
			std::abort();
		}

		in->close();
		return image;
	}

	auto Image::to_path(std::string_view path) -> void {
		auto type = stride == 1 ? OIIO::TypeDesc::UINT8 : OIIO::TypeDesc::FLOAT;
		auto spec = OIIO::ImageSpec{
			i32(width),
			i32(height),
			i32(channels),
			type
		};

		spec.attribute("planarconfig", "contig");
		spec.attribute("oiio::ColorSpace", from_color_space(color_space));

		auto out = OIIO::ImageOutput::create(std::string{path});
		if (!out || !out->open(std::string{path}, spec)) {
			std::printf("can not create image file %s\n", path.data());
			std::abort();
		}

		auto success = out->write_image(type, pixels.data());
		if (!success) {
			std::printf("can not write image %s\n", path.data());
			std::abort();
		}

		out->close();
	}
}
