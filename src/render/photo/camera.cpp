#include <metatron/render/photo/camera.hpp>

namespace metatron::render {
	auto Camera::sample_ray(math::Vector<f32, 2> const& p) -> Ray {
		return {{0.f}, {0.f, 0.f, 1.f}, 0.f};
	}
}
