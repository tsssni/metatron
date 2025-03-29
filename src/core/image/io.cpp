#include <metatron/core/image/io.hpp>
#include <metatron/core/color/color-space.hpp>
#include <tinyexr.h>
#include <stb_image.h>
#include <cstring>

namespace metatron::image {
	auto Stb_Image::from_path(std::string_view path, bool linear) -> std::unique_ptr<Image> {
		auto [width, height, channels] = std::tuple<i32, i32, i32>{0, 0, 0};
		auto* data = stbi_load(path.data(), &width, &height, &channels, STBI_rgb_alpha);
		
		if (!data) {
			std::printf("metatron: failed to load stb image %s: %s\n", path.data(), stbi_failure_reason());
			std::abort();
		}
		
		auto size = math::Vector<usize, 4>{
			usize(width),
			usize(height),
			usize(channels),
			1
		};
		auto image = std::make_unique<Image>(size, color::Color_Space::sRGB.get());

		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				auto pixel = math::Vector<f32, 4>{};
				auto idx = (j * width + i) * 4;
				for (int c = 0; c < channels; c++) {
					pixel[c] = linear
						? f32(data[idx + c]) / 255.0f
						: color::Color_Space::sRGB->decode(f32(data[idx + c]) / 255.0f);
				}
				
				(*image)[i, j] = pixel;
			}
		}
		
		stbi_image_free(data);
		return image;
	}

	auto Exr_Image::from_path(std::string_view path, bool linear) -> std::unique_ptr<Image> {
		EXRHeader header;
		InitEXRHeader(&header);
		
		EXRVersion version;
		const char* err = nullptr;
		
		int ret = ParseEXRVersionFromFile(&version, path.data());
		if (ret != TINYEXR_SUCCESS) {
			std::printf("metatron: failed to parse exr version\n");
			std::abort();
		}
		
		ret = ParseEXRHeaderFromFile(&header, &version, path.data(), &err);
		if (ret != TINYEXR_SUCCESS) {
			std::printf("metatron: %s", err);
			FreeEXRErrorMessage(err);
			std::abort();
		}
		
		EXRImage exr_image;
		InitEXRImage(&exr_image);
		
		ret = LoadEXRImageFromFile(&exr_image, &header, path.data(), &err);
		if (ret != TINYEXR_SUCCESS) {
			std::printf("metatron: %s", err);
			FreeEXRHeader(&header);
			FreeEXRImage(&exr_image);
			FreeEXRErrorMessage(err);
			std::abort();
		}
		
		auto size = math::Vector<usize, 4>{
			usize(exr_image.width),
			usize(exr_image.height),
			usize(exr_image.num_channels),
			usize(4)
		};
		auto image = std::make_unique<Image>(size, color::Color_Space::sRGB.get());
		
		for (auto j = 0; j < exr_image.height; j++) {
			for (auto i = 0; i < exr_image.width; i++) {
				auto pixel = math::Vector<f32, 4>{0.0f, 0.0f, 0.0f, 1.0f};
				
				for (auto c = 0; c < exr_image.num_channels && c < 4; c++) {
					const char* channel_names[4] = {"R", "G", "B", "A"};
					
					auto channel_index = -1;
					for (auto k = 0; k < header.num_channels; k++) {
						if (strcmp(header.channels[k].name, channel_names[c]) == 0) {
							channel_index = k;
							break;
						}
					}
					
					if (channel_index >= 0) {
						auto channel_data = (float const*)(exr_image.images[channel_index]);
						pixel[c] = channel_data[j * exr_image.width + i];
					}
				}
				
				(*image)[i, j] = pixel;
			}
		}
		
		FreeEXRHeader(&header);
		FreeEXRImage(&exr_image);
		
		return image;
	}

	auto Exr_Image::to_path(std::string_view path, Image const& image) -> void {
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
			strncpy(channels_info[c].name, default_channel_names[channels - 1 - c], 255);
		}
		header.channels = channels_info.data();

		auto pixel_type = TINYEXR_PIXELTYPE_FLOAT;
		auto pixel_types = std::vector<i32>(channels, pixel_type);
		auto requested_pixel_types = std::vector<i32>(channels, pixel_type);
		header.pixel_types = pixel_types.data();
		header.requested_pixel_types = requested_pixel_types.data();
		header.compression_type = TINYEXR_COMPRESSIONTYPE_ZIP;

		auto image_ptr = std::vector<f32*>(channels);
		for (auto c = 0; c < channels; c++) {
			image_ptr[c] = channel_data[channels - 1 - c].data();
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
}
