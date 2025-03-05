#include <metatron/geometry/shape/sphere.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::shape {
	Sphere::Sphere(f32 radius, f32 theta_min, f32 theta_max, f32 phi_max)
		: radius(radius), theta_min(theta_min), theta_max(theta_max), phi_max(phi_max) {}

	auto Sphere::bounding_box(usize idx) const -> intr::Bounding_Box {
		return {
			{-radius},
			{radius}
		};
	}

	auto Sphere::sample(
		intr::Context const& ctx,
		math::Vector<f32, 2> const& u,
		usize idx
	) const -> std::optional<intr::Interaction> {
		return {};
	}

	auto Sphere::intersect(
		math::Ray const& r,
		usize idx
	) const -> std::optional<intr::Interaction> {
		auto a = math::dot(r.d, r.d);
		auto b = math::dot(r.o, r.d) * 2.f;
		auto c = math::dot(r.o, r.o);

		auto delta = b * b - 4.f * a * c;
		if (delta < 0.f) return {};

		auto x0 = (-b - std::sqrt(delta)) / (2.f * a);
		auto x1 = (-b + std::sqrt(delta)) / (2.f * a);
		if (x1 < 0.f) return {};

		auto t = x0 < 0.f ? x1 : x0;
		auto p = r.o + t * r.d;
		auto n = math::normalize(p);

		auto theta = std::acos(n[1]);
		auto phi = std::atan2(n[2], n[0]);
		auto uv = math::Vector<f32, 2>{
			theta / math::pi,
			(phi < 0.f ? 2.f * math::pi : phi) / 2.f * math::pi
		};

		return intr::Interaction{p, n, uv, 1.f};
	}

}
