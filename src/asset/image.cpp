#include <metatron/asset/image.hpp>
#include <tinyexr.h>
#include <span>
#include <cstring>

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

	auto Image::from_path(std::string_view path) -> std::unique_ptr<Image> {
		return std::make_unique<Image>(math::Vector<usize, 4>{0});
	}

	auto exr_writer(std::string_view path, Image const& image) -> void {
		auto width = image.size[0];
		auto height = image.size[1];
		auto channels = image.size[2];
		auto stride = image.size[3];

		auto channel_data = std::vector<std::vector<f32>>(channels, std::vector<f32>(width * height));
		for (auto j = 0; j < height; j++) {
			for (auto i = 0; i < width; i++) {
				auto pixel = math::Vector<f32, 4>{image[i, j]};
				for (auto c = 0; c < channels; c++) {
					channel_data[c][j * width + i] = pixel[c];
				} 
			}
		}

		auto header = EXRHeader{};
		InitEXRHeader(&header);

		header.num_channels = channels;
		auto channels_info = std::vector<EXRChannelInfo>(channels);
		auto default_channel_names = std::array<char const*, 4>{"R", "G", "B", "A"};
		for (auto c = 0; c < channels; c++) {
			strncpy(channels_info[c].name, default_channel_names[c], 255);
		}
		header.channels = channels_info.data();

		auto pixel_type = stride > 2 ? TINYEXR_PIXELTYPE_FLOAT : TINYEXR_PIXELTYPE_HALF;
		auto pixel_types = std::vector<i32>(channels, pixel_type);
		auto requested_pixel_types = std::vector<i32>(channels, pixel_type);
		header.pixel_types = pixel_types.data();
		header.requested_pixel_types = requested_pixel_types.data();
		header.compression_type = TINYEXR_COMPRESSIONTYPE_ZIP;

		auto image_ptr = std::vector<f32*>(channels);
		for (auto c = 0; c < channels; c++) {
			image_ptr[c] = channel_data[c].data();
		}

		EXRImage exr_image;
		InitEXRImage(&exr_image);
		exr_image.width = width;
		exr_image.height = height;
		exr_image.num_channels = channels;
		exr_image.images = reinterpret_cast<byte**>(image_ptr.data());

		auto* err = (char const*)(nullptr);
		auto ret = SaveEXRImageToFile(&exr_image, &header, path.data(), &err);
		if (ret != TINYEXR_SUCCESS) {
			std::printf("metatron: %s", err);
			std::abort();
		}
	}

	auto Image::to_path(std::string_view path) -> void {
		if (path.ends_with(".exr")) {
			exr_writer(path, *this);
		}
	}
}
