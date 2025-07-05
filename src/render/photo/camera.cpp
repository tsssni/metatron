#include <metatron/render/photo/camera.hpp>
#include <metatron/core/stl/optional.hpp>

namespace mtt::photo {
	Camera::Camera(
		std::unique_ptr<Film> film,
		std::unique_ptr<Lens> lens
	): film(std::move(film)), lens(std::move(lens)) {
		auto& ray = default_differential;
		auto r_pos = math::Vector<f32, 3>{0.f};
		auto rx_pos = r_pos + math::Vector<f32, 3>{this->film->dxdy[0], 0.f, 0.f};
		auto ry_pos = r_pos + math::Vector<f32, 3>{0.f, this->film->dxdy[1], 0.f};

		MTT_OPT_OR_RETURN(r_intr, this->lens->sample(r_pos, {0.f}));
		MTT_OPT_OR_RETURN(rx_intr, this->lens->sample(rx_pos, {0.f}));
		MTT_OPT_OR_RETURN(ry_intr, this->lens->sample(ry_pos, {0.f}));

		ray.differentiable = false;
		ray.r = r_intr.r;
		ray.rx = rx_intr.r;
		ray.ry = ry_intr.r;
	}

	auto Camera::sample(
		math::Vector<usize, 2> pixel,
		usize idx,
		math::Sampler& sampler
	) -> std::optional<Interaction> {
		sampler.start(pixel, idx);
		auto fixel = (*film)(pixel, sampler.generate_pixel_2d());

		auto ray = math::Ray_Differential{};
		auto r_pos = math::Vector<f32, 3>{fixel.position, 0.f};
		auto rx_pos = r_pos + math::Vector<f32, 3>{fixel.dxdy[0], 0.f, 0.f};
		auto ry_pos = r_pos + math::Vector<f32, 3>{0.f, fixel.dxdy[1], 0.f};

		MTT_OPT_OR_RETURN(r_intr, lens->sample(r_pos, sampler.generate_2d()), {});
		MTT_OPT_OR_RETURN(rx_intr, lens->sample(rx_pos, sampler.generate_2d()), {});
		MTT_OPT_OR_RETURN(ry_intr, lens->sample(ry_pos, sampler.generate_2d()), {});

		ray.differentiable = true;
		ray.r = r_intr.r;
		ray.rx = rx_intr.r;
		ray.ry = ry_intr.r;

		return Interaction{ray, default_differential, fixel};
	}

	auto Camera::to_path(std::string_view path) -> void {
		film->to_path(path);
	}
}
