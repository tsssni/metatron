#include <metatron/render/photo/film.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::photo {
	Fixel::Fixel(Film* film, math::Vector<f32, 2> const& position, f32 weight)
		: film(film), position(position), weight(weight) {}

	auto Fixel::operator=(spectra::Spectrum const& spectrum) -> void {
		auto uv = (position / film->size) + 0.5f;
		auto pixel = math::Vector<i32, 2>{
			i32(uv[0] * film->image.size[0]),
			i32(uv[1] * film->image.size[1])
		};
		auto rgba = math::Vector<f32, 4>{(*(film->sensor))(spectrum)};
		rgba[3] = weight;
		film->image[pixel[0], pixel[1]] += rgba;
	}

	Film::Film(
		math::Vector<f32, 2> const& film_size,
		math::Vector<usize, 2> const& image_size,
		std::unique_ptr<Sensor> sensor,
		std::unique_ptr<math::Filter> filter
	):
	size(film_size),
	image({image_size[0], image_size[1], 4uz, 4uz}),
	sensor(std::move(sensor)),
	filter(std::move(filter)) {}

	auto Film::operator()(math::Vector<f32, 2> pixel_position) -> std::optional<film::Interaction> {
		auto uv = math::Vector<f32, 2>{
			pixel_position[0] / image.size[0],
			pixel_position[1] / image.size[1],
		};
		if (uv[0] < 0.f || uv[0] >= 1.f || uv[1] < 0.f || uv[1] >= 1.f) return {};

		auto film_position = math::Vector<f32, 2>{
			(uv[0] - 0.5f) * size[0],
			(uv[1] - 0.5f) * size[1]
		};
		auto weight = (*filter)({
			std::fmod(pixel_position[0], 1.f) - 0.5f,
			std::fmod(pixel_position[1], 1.f) - 0.5f,
		});

		return film::Interaction{
			{
				this, 
				film_position,
				weight
			}
		};
	}

	auto Film::to_path(std::string_view path) -> void {
		for (auto j = 0; j < image.size[1]; j++) {
			for (auto i = 0; i < image.size[0]; i++) {
				auto pixel = math::Vector<f32, 4>{image[i, j]};
				pixel = std::abs(pixel[3]) < math::epsilon ? math::Vector<f32, 4>{0.f} : pixel / pixel[3];
				image[i, j] = pixel;
			}
		}
		image.to_path(path);
	}
}
