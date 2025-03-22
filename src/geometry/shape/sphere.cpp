#include <metatron/geometry/shape/sphere.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/distribution/sphere.hpp>
#include <metatron/core/math/distribution/cone.hpp>

namespace metatron::shape {
	Sphere::Sphere(f32 r)
		: r(r) {}

	auto Sphere::bounding_box(usize idx) const -> math::Bounding_Box {
		return {
			{-r},
			{r}
		};
	}

	auto Sphere::operator()(
		math::Ray const& r,
		usize idx
	) const -> std::optional<Interaction> {
		auto a = math::dot(r.d, r.d);
		auto b = math::dot(r.o, r.d) * 2.f;
		auto c = math::dot(r.o, r.o) - this->r * this->r;

		auto delta = b * b - 4.f * a * c;
		if (delta < 0.f) return {};

		auto x0 = (-b - std::sqrt(delta)) / (2.f * a);
		auto x1 = (-b + std::sqrt(delta)) / (2.f * a);
		if (x1 < 0.f) return {};

		auto t = x0 < 0.f ? x1 : x0;
		auto p = r.o + t * r.d;
		auto n = math::normalize(p);

		auto s = math::cartesion_to_sphere(n);
		auto uv = math::Vector<f32, 2>{
			s[0] / math::pi,
			s[1] / (2.f * math::pi)
		};

		return Interaction{p, n, uv, {}, {}, t, 1.f};
	}

	auto Sphere::operator()(
		math::Ray_Differential const& rd,
		usize idx
	) const -> std::optional<Interaction> {
		return {};
	}

	auto Sphere::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u,
		usize idx
	) const -> std::optional<Interaction> {
		auto d = math::length(ctx.p);
		if (d <= r) {
			auto dist = math::Sphere_Distribution{};
			auto p = dist.sample(u);
			auto n  = p;

			auto s = math::cartesion_to_sphere(n);
			auto uv = math::Vector<f32, 2>{
				s[0] / math::pi,
				s[1] / (2.f * math::pi)
			};

			return Interaction{p, n, uv, {}, {}, 0.f, dist.pdf()};
		} else {
			auto cos_theta_max = std::sqrt(1.f - (r * r) / (d * d));
			auto dist = math::Cone_Distribution{cos_theta_max};
			auto dir = math::normalize(-ctx.p);

			auto sdir = dist.sample(u);
			auto rot = math::Quaternion<f32>::from_rotation_between({0.f, 0.f, 1.f}, dir);
			sdir = math::rotate(math::Vector<f32, 4>{sdir, 0.f}, rot);

			return (*this)(math::Ray{ctx.p, sdir});
		}
	}

}
