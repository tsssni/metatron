#pragma once
#include <metatron/render/photo/sensor.hpp>
#include <metatron/resource/image/image.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/math/filter/filter.hpp>
#include <metatron/core/math/vector.hpp>
#include <memory>

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
		);
		auto operator=(spectra::Stochastic_Spectrum const& spectrum) -> void;
		auto operator=(spectra::Spectrum const& spectrum) -> void;
	private:
		Film* film;
	};

	struct Film final {
		math::Vector<f32, 2> size;
		math::Vector<f32, 2> dxdy;

		Film(
			math::Vector<f32, 2> const& film_size,
			math::Vector<usize, 2> const& image_size,
			std::unique_ptr<Sensor> sensor,
			std::unique_ptr<math::Filter> filter,
			color::Color_Space const* color_space
		);
		auto operator()(
			math::Vector<usize, 2> pixel,
			math::Vector<f32, 2> u
		) -> Fixel;
		auto to_path(std::string_view path) -> void;

	private:
		friend Fixel;
		image::Image image;
		std::unique_ptr<Sensor> sensor;
		std::unique_ptr<math::Filter> filter;
	};
}
