#include <metatron/core/math/filter/box.hpp>

namespace metatron::math {
	Box_Filter::Box_Filter(math::Vector<f32, 2> const& radius): radius(radius) {}

	auto Box_Filter::operator()(math::Vector<f32, 2> const& p) const -> f32 {
		return (std::abs(p[0]) < radius[0] && std::abs(p[1]) < radius[1]) ? 1.f : 0.f;
	}

	auto Box_Filter::sample(math::Vector<f32, 2> const& u) const -> math::Vector<f32, 2> {
		return {std::lerp(-radius[0], radius[0], u[0]), std::lerp(-radius[1], radius[1], u[1])};
	}
}
