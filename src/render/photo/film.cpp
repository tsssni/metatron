#include <metatron/render/photo/film.hpp>

namespace metatron::photo {
	Film::Fixel::Fixel(Film* film, math::Vector<f32, 2> const& position, f32 weight)
		: film(film), position(position), weight(weight) {}

	auto Film::Fixel::operator=(spectra::Spectrum const& spectrum) -> void {
		auto uv = ((position / film->size) * 0.5 + 0.5);
		auto pixel = math::Vector<i32, 2>{
			i32(uv[0] * film->image.size[0]),
			i32(uv[1] * film->image.size[1])
		};
		film->image[pixel[0], pixel[1]] += weight * math::Vector<f32, 3>{spectrum};
	}

	Film::Film(
		math::Vector<f32, 2> const& film_size,
		math::Vector<usize, 2> const& image_size,
		std::unique_ptr<Sensor> sensor,
		std::unique_ptr<math::Filter> filter
	):
	size(film_size),
	image({image_size[0], image_size[1], 4, 4}),
	sensor(std::move(sensor)),
	filter(std::move(filter)) {}

	auto Film::sample(math::Vector<f32, 2> pixel_position) -> Fixel {
		auto uv = math::Vector<f32, 2>{
			pixel_position[0] / image.size[0],
			pixel_position[1] / image.size[1],
		};
		auto film_position = math::Vector<f32, 2>{
			uv[0] * size[0],
			uv[1] * size[1]
		} * 2.0 - 1.0;

		return Fixel{
			this, 
			film_position,
			1.f
		};
	}
}
