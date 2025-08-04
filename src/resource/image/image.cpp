#include <metatron/resource/image/image.hpp>
#include <OpenImageIO/imageio.h>
#include <print>

namespace mtt::image {
	Image::Pixel::Pixel(Image const* image, byte* start) noexcept
	: image(image), start(start) {}

	Image::Pixel::operator math::Vector<f32, 4>() const noexcept {
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

	auto Image::Pixel::operator=(math::Vector<f32, 4> const& v) noexcept -> void {
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

	auto Image::Pixel::operator+=(math::Vector<f32, 4> const& v) noexcept -> void {
		*this = math::Vector<f32, 4>(*this) + v;
	}

	Image::Image(
		math::Vector<usize, 4> const& size,
		color::Color_Space const* color_space,
		bool linear
	) noexcept:
	size(size),
	pixels(size[0] * size[1] * size[2] * size[3]),
	color_space(color_space),
	linear(linear) {}

	auto Image::operator[](usize x, usize y) noexcept -> Pixel {
		auto offset = (y * width + x) * channels * stride;
		return Pixel{this, &pixels[offset]};
	}

	auto Image::operator[](usize x, usize y) const noexcept -> Pixel const {
		auto offset = (y * width + x) * channels * stride;
		return (Pixel const){this, const_cast<byte*>(&pixels[offset])};
	}

	auto Image::from_path(
		std::string_view path,
		bool linear
	) noexcept -> poly<Image> {
		auto in = OIIO::ImageInput::open(std::string{path});
		if (!in) {
			std::println("cannot find image {}", path);
			std::abort();
		}

		auto& spec = in->spec();
		auto cs_name = spec.get_string_attribute("oiio:ColorSpace");
		auto* color_space = color::Color_Space::color_spaces.contains(cs_name)
		? color::Color_Space::color_spaces.at(cs_name)
		: color::Color_Space::color_spaces.at("sRGB");

		auto image = make_poly<Image>(
			math::Vector<usize, 4>{
				usize(spec.width),
				usize(spec.height),
				usize(spec.nchannels),
				spec.format.size()
			},
			color_space,
			linear
		);

		auto success = in->read_image(0, 0, 0, spec.nchannels, spec.format, image->pixels.data());
		if (!success) {
			std::println("can not read image {}", path);
			std::abort();
		}

		in->close();
		return image;
	}

	auto Image::to_path(std::string_view path) const noexcept -> void {
		auto type = stride == 1 ? OIIO::TypeDesc::UINT8 : OIIO::TypeDesc::FLOAT;
		auto spec = OIIO::ImageSpec{
			i32(width),
			i32(height),
			i32(channels),
			type
		};

		auto cs_name = std::string{"sRGB"};
		for (auto const& [name, cs]: color::Color_Space::color_spaces) {
			if (color_space == cs) {
				cs_name = name;
			}
		}
		spec.attribute("oiio::ColorSpace", cs_name);
		spec.attribute("planarconfig", "contig");

		auto out = OIIO::ImageOutput::create(std::string{path});
		if (!out || !out->open(std::string{path}, spec)) {
			std::println("can not create image file {}", path);
			std::abort();
		}

		auto success = out->write_image(type, pixels.data());
		if (!success) {
			std::println("can not write image {}", path);
			std::abort();
		}

		out->close();
	}
}
