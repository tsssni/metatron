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
		auto pixel_position = math::Vector<f32, 2>{
			pixel[0] + 0.5f,
			pixel[1] + 0.5f
		};
		auto fixel = film->sample(pixel_position);
		auto ray = lens->sample({pixel_position[0], pixel_position[1], 0.f}, {0.f, 0.f});
		return {ray, fixel};
	}

	auto Camera::to_path(std::string_view path) -> void {
		film->to_path(path);
	}
}
