#pragma once
#include <metatron/core/math/vector.hpp>

namespace metatron::math {
	auto inline cartesion_to_sphere(math::Vector<f32, 3> const& d) -> math::Vector<f32, 2> {
		auto theta = std::acos(d[1]);
		auto phi = std::atan2(d[2], d[0]);
		return {theta, phi};
	}

	auto inline sphere_to_cartesion(math::Vector<f32, 2> const& s) -> math::Vector<f32, 3> {
		auto theta = s[0];
		auto phi = s[1];
		auto x = std::sin(theta) * std::cos(phi);
		auto y = std::sin(theta) * std::sin(phi);
		auto z = std::cos(theta);
		return {x, y, z};
	}
}
