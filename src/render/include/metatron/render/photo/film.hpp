#pragma once
#include <metatron/render/photo/sensor.hpp>
#include <metatron/resource/image/image.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/math/filter/filter.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::photo {
	struct Camera;
	struct Film;

	struct Fixel final {
		math::Vector<usize, 2> const pixel;
		math::Vector<f32, 2> const position;
		math::Vector<f32, 2> const dxdy;
		f32 const weight;
		Fixel(
			Film* film,
			math::Vector<usize, 2> const& pixel,
			math::Vector<f32, 2> const& position,
			f32 weight
		) noexcept;
		auto operator=(spectra::Stochastic_Spectrum const& spectrum) noexcept -> void;
	private:
		Film* film;
	};

	struct Film final {
		math::Vector<f32, 2> size;
		math::Vector<f32, 2> dxdy;

		Film(
			math::Vector<f32, 2> const& film_size,
			math::Vector<usize, 2> const& image_size,
			view<math::Filter> filter,
			Sensor const* sensor,
			color::Color_Space const* color_space
		) noexcept;
		auto operator()(
			math::Vector<usize, 2> pixel,
			math::Vector<f32, 2> u
		) noexcept -> Fixel;
		auto to_path(std::string_view path) noexcept -> void;

	private:
		friend Fixel;
		image::Image image;
		view<math::Filter> filter;
		Sensor const* sensor;
	};
}
