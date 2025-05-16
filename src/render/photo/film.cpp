#include <metatron/render/photo/film.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <span>

namespace metatron::photo {
	Fixel::Fixel(
		Film* film,
		math::Vector<usize, 2> const& pixel,
		math::Vector<f32, 2> const& position,
		f32 weight
	):
	film(film),
	pixel(pixel),
	position(position),
	dxdy(math::foreach(
		position + film->dxdy - film->size / 2.f,
		[](f32 x, usize i){return x < 0.f ? 1.f : -1.f;}
	) * film->dxdy),
	weight(weight) {}

	auto Fixel::operator=(spectra::Stochastic_Spectrum const& spectrum) -> void {
		auto rgb = (*(film->sensor))(spectrum);
		film->image[pixel[0], pixel[1]] += {rgb * weight, 1.f};
		// film->image[pixel[0], pixel[1]] += {math::Vector<f32, 3>{std::span(spectrum.value)}, 1.f};
	}

	Film::Film(
		math::Vector<f32, 2> const& film_size,
		math::Vector<usize, 2> const& image_size,
		std::unique_ptr<Sensor> sensor,
		std::unique_ptr<math::Filter> filter,
		color::Color_Space const* color_space
	):
	size(film_size),
	dxdy(film_size / image_size),
	image({image_size, 4uz, 4uz}, color_space),
	sensor(std::move(sensor)),
	filter(std::move(filter)) {}

	auto Film::operator()(
		math::Vector<usize, 2> pixel,
		math::Vector<f32, 2> u
	) -> Fixel {
		auto f_intr = filter->sample(u).value();
		auto pixel_position = math::Vector<f32, 2>{pixel} + 0.5f + f_intr.p;
		auto uv = pixel_position / image.size;
		auto film_position = (uv - 0.5f) * math::Vector<f32, 2>{-1.f, 1.f} * size;

		return {
			this,
			pixel,
			film_position,
			math::guarded_div(f_intr.weight, f_intr.pdf)
		};
	}

	auto Film::to_path(std::string_view path) -> void {
		for (auto j = 0; j < image.height; j++) {
			for (auto i = 0; i < image.width; i++) {
				auto pixel = math::Vector<f32, 4>{image[i, j]};
				pixel = std::abs(pixel[3]) < math::epsilon<f32> ? math::Vector<f32, 4>{0.f} : pixel / pixel[3];
				image[i, j] = pixel;
			}
		}
		image.to_path(path);
	}
}
