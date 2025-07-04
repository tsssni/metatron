#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace metatron::math {
	auto inline constexpr cartesion_to_unit_sphere(math::Vector<f32, 3> const& d) -> math::Vector<f32, 2> {
		auto theta = std::acos(d[1]);
		auto phi = std::atan2(d[2], d[0]);
		if (phi < 0.f) {
			phi += 2.f * math::pi;
		}
		return {theta, phi};
	}

	auto inline constexpr unit_sphere_to_cartesion(f32 cos_theta, f32 phi) -> math::Vector<f32, 3> {
		auto sin_theta_2 = 1.f - cos_theta * cos_theta;
		auto sin_theta = math::sqrt(sin_theta_2);
		auto x = sin_theta * std::cos(phi);
		auto z = sin_theta * std::sin(phi);
		auto y = cos_theta;
		return {x, y, z};
	}

	auto inline constexpr unit_sphere_to_cartesion(math::Vector<f32, 2> const& s) -> math::Vector<f32, 3> {
		auto [theta, phi] = s;
		return unit_sphere_to_cartesion(std::cos(theta), phi);
	}

	auto inline constexpr cartesion_to_sphere(math::Vector<f32, 3> const& d) -> math::Vector<f32, 2> {
		auto r = math::length(d);
		auto nd = math::normalize(d);
		auto [theta, phi] = cartesion_to_unit_sphere(nd);
		return {r, theta, phi};
	}

	auto inline constexpr sphere_to_cartesion(f32 r, f32 cos_theta, f32 phi) -> math::Vector<f32, 3> {
		auto c = unit_sphere_to_cartesion(cos_theta, phi);
		return r * c;
	}

	auto inline constexpr sphere_to_cartesion(math::Vector<f32, 3> const& s) -> math::Vector<f32, 3> {
		auto r = s[0];
		auto theta = s[1];
		auto phi = s[2];
		return sphere_to_cartesion(r, std::cos(theta), phi);
	}

	auto inline constexpr unit_to_cos_theta(math::Vector<f32, 3> const& d) -> f32 {
		return d[1];
	}

	auto inline constexpr unit_to_cos2_theta(math::Vector<f32, 3> const& d) -> f32 {
		return d[1] * d[1];
	}

	auto inline constexpr unit_to_sin2_theta(math::Vector<f32, 3> const& d) -> f32 {
		return std::max(0.f, 1.f - d[1] * d[1]);
	}

	auto inline constexpr unit_to_sin_theta(math::Vector<f32, 3> const& d) -> f32 {
		return math::sqrt(unit_to_sin2_theta(d));
	}

	auto inline constexpr unit_to_tan2_theta(math::Vector<f32, 3> const& d) -> f32 {
		return unit_to_sin2_theta(d) / unit_to_cos2_theta(d);
	}

	auto inline constexpr unit_to_tan_theta(math::Vector<f32, 3> const& d) -> f32 {
		return unit_to_sin_theta(d) / unit_to_cos_theta(d);
	}

	auto inline constexpr unit_to_cos_phi(math::Vector<f32, 3> const& d) -> f32 {
		auto sin_theta = unit_to_sin_theta(d);
		return std::clamp(math::guarded_div(d[0], sin_theta), -1.f, 1.f);
	}

	auto inline constexpr unit_to_sin_phi(math::Vector<f32, 3> const& d) -> f32 {
		auto sin_theta = unit_to_sin_theta(d);
		return std::clamp(math::guarded_div(d[2], sin_theta), -1.f, 1.f);
	}
}
