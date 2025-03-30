#pragma once
#include <metatron/render/photo/film.hpp>
#include <metatron/render/photo/lens/lens.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/sampler/sampler.hpp>
#include <memory>

namespace metatron::photo {
	struct Interaction final {
		math::Ray_Differential ray;
		Fixel fixel;
	};

	struct Camera final {
		Camera(
			std::unique_ptr<Film> film,
			std::unique_ptr<Lens> lens
		);
		auto sample(
			math::Vector<usize, 2> pixel,
			usize idx,
			math::Sampler& sampler
		) -> std::optional<Interaction>;
		auto to_path(std::string_view path) -> void;

	private:
		std::unique_ptr<Film> film;
		std::unique_ptr<Lens> lens;
	};
}
