#include <metatron/render/photo/film.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::photo {
	Fixel::Fixel(Film* film, math::Vector<f32, 2> const& position, f32 weight):
		film(film),
		dxdy(math::foreach(
			position + film->dxdy - film->size / 2.f,
			[](f32 x){return x < 0.f ? 1.f : -1.f;}
		) * film->dxdy),
		position(position),
		weight(weight) {}

	auto Fixel::operator=(spectra::Stochastic_Spectrum const& spectrum) -> void {
		auto uv = (position / film->size) + 0.5f;
		auto pixel = math::Vector<i32, 2>{uv * film->image.size};
		auto rgb = (*(film->sensor))(spectrum);
		film->image[pixel[0], pixel[1]] += {rgb, weight};
	}

	Film::Film(
		math::Vector<f32, 2> const& film_size,
		math::Vector<usize, 2> const& image_size,
		std::unique_ptr<Sensor> sensor,
		std::unique_ptr<math::Filter> filter,
		color::Color_Space const* color_space
	):
	size(film_size),
	dxdy(math::Vector<f32, 2>{1.f} / image_size / film_size),
	image({image_size, 4uz, 4uz}, color_space),
	sensor(std::move(sensor)),
	filter(std::move(filter)) {}

	auto Film::operator()(math::Vector<f32, 2> pixel_position) -> Fixel {
		auto uv = pixel_position / image.size;
		auto film_position = (uv - 0.5f) * size;
		auto weight = (*filter)(math::mod(pixel_position, 1.f) - 0.5f);

		return {
			this,
			film_position,
			weight
		};
	}

	auto Film::to_path(std::string_view path) -> void {
		for (auto j = 0; j < image.size[1]; j++) {
			for (auto i = 0; i < image.size[0]; i++) {
				auto pixel = math::Vector<f32, 4>{image[i, j]};
				pixel = std::abs(pixel[3]) < math::epsilon<f32> ? math::Vector<f32, 4>{0.f} : pixel / pixel[3];
				image[i, j] = pixel;
			}
		}
		image.to_path(path);
	}
}
