#include <metatron/render/photo/camera.hpp>
#include <metatron/core/stl/optional.hpp>

namespace metatron::photo {
	Camera::Camera(
		std::unique_ptr<Film> film,
		std::unique_ptr<Lens> lens
	): film(std::move(film)), lens(std::move(lens)) {}

	auto Camera::sample(
		math::Vector<usize, 2> pixel,
		usize idx,
		math::Sampler& sampler
	) -> std::optional<Interaction> {
		sampler.start(pixel, idx);
		auto sample = sampler.generate_pixel_2d();
		auto pixel_position = sample + pixel;
		auto fixel = (*film)(pixel_position);

		auto ray = math::Ray_Differential{};
		ray.differentiable = true;
		OPTIONAL_OR_RETURN(r_intr, lens->sample({fixel.position, 0.f}, sampler.generate_2d()), {});
		ray.r = r_intr.r;

		for (auto dx: {1.f, -1.f}) {
			auto fixel_dx = fixel.position;
			fixel_dx[0] += dx;
			if (fixel_dx < math::Vector<f32, 2>{0.f} || fixel_dx >= math::Vector<f32, 2>{1.f}) {
				continue;
			}
			OPTIONAL_OR_RETURN(rx_intr, lens->sample({fixel_dx, 0.f}, sampler.generate_2d()), {});
			ray.rx = rx_intr.r;
		}

		for (auto dy: {1.f, -1.f}) {
			auto fixel_dy = fixel.position;
			fixel_dy[1] += dy;
			if (fixel_dy < math::Vector<f32, 2>{0.f} || fixel_dy >= math::Vector<f32, 2>{1.f}) {
				continue;
			}
			OPTIONAL_OR_RETURN(ry_intr, lens->sample({fixel_dy, 0.f}, sampler.generate_2d()), {});
			ray.ry = ry_intr.r;
		}

		return Interaction{ray, fixel};
	}

	auto Camera::to_path(std::string_view path) -> void {
		film->to_path(path);
	}
}
