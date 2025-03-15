#include <metatron/geometry/shape/sphere.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sphere.hpp>

namespace metatron::shape {
	Sphere::Sphere(f32 radius, f32 theta_min, f32 theta_max, f32 phi_max)
		: radius(radius), theta_min(theta_min), theta_max(theta_max), phi_max(phi_max) {}

	auto Sphere::bounding_box(usize idx) const -> math::Bounding_Box {
		return {
			{-radius},
			{radius}
		};
	}

	auto Sphere::operator()(
		math::Ray const& r,
		usize idx
	) const -> std::optional<Interaction> {
		auto a = math::dot(r.d, r.d);
		auto b = math::dot(r.o, r.d) * 2.f;
		auto c = math::dot(r.o, r.o) - radius * radius;

		auto delta = b * b - 4.f * a * c;
		if (delta < 0.f) return {};

		auto x0 = (-b - std::sqrt(delta)) / (2.f * a);
		auto x1 = (-b + std::sqrt(delta)) / (2.f * a);
		if (x1 < 0.f) return {};

		auto t = x0 < 0.f ? x1 : x0;
		auto p = r.o + t * r.d;
		auto n = math::normalize(p);

		auto sphere_coord = math::cartesion_to_sphere(n);
		auto theta = sphere_coord[0];
		auto phi = sphere_coord[1];
		auto uv = math::Vector<f32, 2>{
			theta / math::pi,
			(phi < 0.f ? 2.f * math::pi : phi) / 2.f * math::pi
		};

		return Interaction{p, n, uv, 1.f};
	}

	auto Sphere::sample(
		Context const& ctx,
		math::Vector<f32, 2> const& u,
		usize idx
	) const -> std::optional<Interaction> {
		return {};
	}

}
