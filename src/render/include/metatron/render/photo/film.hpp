#pragma once
#include <metatron/render/photo/sensor.hpp>
#include <metatron/render/photo/image.hpp>
#include <metatron/core/math/sampler.hpp>
#include <metatron/core/math/filter.hpp>
#include <metatron/core/math/vector.hpp>
#include <memory>

namespace metatron::photo {
	struct Camera;

	struct Film final {
		struct Fixel final {
			math::Vector<f32, 2> const position;
			f32 const weight;
			Fixel(Film* film, math::Vector<f32, 2> const& position, f32 weight);
			auto operator=(spectra::Spectrum const& spectrum) -> void;
		private:
			Film* film;
		};

		Film(
			math::Vector<f32, 2> const& film_size,
			math::Vector<usize, 2> const& image_size,
			std::unique_ptr<Sensor> sensor,
			std::unique_ptr<math::Filter> filter
		);
		auto sample(math::Vector<f32, 2> pixel_position) -> Fixel;
		auto to_path(std::string_view path) -> void;

	private:
		friend Fixel;
		math::Vector<f32, 2> size;
		Image image;
		std::unique_ptr<Sensor> sensor;
		std::unique_ptr<math::Filter> filter;
	};
}
