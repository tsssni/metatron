#include <metatron/resource/photo/film.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::photo {
	Fixel::Fixel(
		mut<Film> film,
		math::Vector<usize, 2> const& pixel,
		math::Vector<f32, 2> const& position,
		f32 weight
	) noexcept:
	film(film),
	pixel(pixel),
	position(position),
	dxdy(math::foreach(
		[](f32 x, usize i){return x < 0.f ? 1.f : -1.f;},
		position + film->dxdy - film->film_size / 2.f
	) * film->dxdy),
	weight(weight) {}

	auto Fixel::operator=(spectra::Stochastic_Spectrum const& spectrum) noexcept -> void {
		auto rgb = (*film->sensor)(spectrum);
		film->image[pixel[0], pixel[1]] += {rgb * weight, 1.f};
		// film->image[pixel[0], pixel[1]] += {math::Vector<f32, 3>{spectrum.value}, 1.f};
	}

	Film::Film(
		math::Vector<f32, 2> const& film_size,
		math::Vector<usize, 2> const& image_size,
		view<math::Filter> filter,
		view<Sensor> sensor,
		view<color::Color_Space> color_space
	) noexcept:
	film_size(film_size),
	image_size(image_size),
	dxdy(film_size / image_size),
	image({image_size, 4uz, 4uz}, color_space),
	filter(filter),
	sensor(sensor) {}

	auto Film::operator()(
		math::Vector<usize, 2> pixel,
		math::Vector<f32, 2> u
	) noexcept -> Fixel {
		auto f_intr = filter->sample(u).value();
		auto pixel_position = math::Vector<f32, 2>{pixel} + 0.5f + f_intr.p;
		auto uv = pixel_position / image.size;
		auto film_position = (uv - 0.5f) * math::Vector<f32, 2>{-1.f, 1.f} * film_size;

		return {
			this,
			pixel,
			film_position,
			math::guarded_div(f_intr.weight, f_intr.pdf)
		};
	}

	auto Film::to_path(std::string_view path) noexcept -> void {
		stl::scheduler::instance().sync_parallel(
			math::Vector<usize, 2>{image.width, image.height},
			[&](math::Vector<usize, 2> const& px) {
				auto [i, j] = px;
				auto pixel = math::Vector<f32, 4>{image[i, j]};
				pixel = math::abs(pixel[3]) < math::epsilon<f32> ? math::Vector<f32, 4>{0.f} : pixel / pixel[3];
				image[i, j] = pixel;
			}
		);
		image.to_path(path);
	}
}
