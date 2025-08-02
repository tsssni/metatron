#pragma once
#include <metatron/render/photo/film.hpp>
#include <metatron/render/lens/lens.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/sampler/sampler.hpp>

namespace mtt::photo {
	struct Interaction final {
		math::Ray_Differential ray_differential;
		math::Ray_Differential default_differential;
		Fixel fixel;
	};

	struct Camera final {
		Camera(
			view<Lens> lens,
			mut<Film> film
		) noexcept;
		auto sample(
			math::Vector<usize, 2> pixel,
			usize idx,
			mut<math::Sampler> sampler
		) const noexcept -> std::optional<Interaction>;
		auto to_path(std::string_view path) -> void;

	private:
		view<Lens> lens;
		mut<Film> film;
		math::Ray_Differential default_differential;
	};
}
