#pragma once
#include <metatron/render/photo/film.hpp>
#include <metatron/render/photo/lens.hpp>
#include <metatron/core/math/sampler.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>
#include <memory>

namespace metatron::photo {
	struct Sample final {
		math::Ray r;
		Film::Fixel fixel;
	};

	struct Camera final {
		Camera(
			std::unique_ptr<Film> film,
			std::unique_ptr<Lens> lens
		);

		auto sample(
			math::Vector<usize, 2> pixel,
			usize idx,
			math::Sampler const& sampler
		) -> Sample;

	private:
		std::unique_ptr<Film> film;
		std::unique_ptr<Lens> lens;
	};
}
