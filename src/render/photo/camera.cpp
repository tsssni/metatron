#include <metatron/render/photo/camera.hpp>

namespace metatron::photo {
	Camera::Camera(
		std::unique_ptr<Film> film,
		std::unique_ptr<Lens> lens
	) : film(std::move(film)), lens(std::move(lens)) {}

	auto Camera::sample(
		math::Vector<usize, 2> pixel,
		usize idx,
		math::Sampler const& sampler
	) -> Sample {
		auto fixel = film->sample(pixel);
		auto ray = lens->sample({fixel.position[0], fixel.position[1], 0.f}, {0.f, 0.f});
		return {ray, fixel};
	}
}
