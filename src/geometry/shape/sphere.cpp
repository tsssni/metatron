#include <metatron/geometry/shape/sphere.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/distribution/sphere.hpp>
#include <metatron/core/math/distribution/cone.hpp>
#include <metatron/core/stl/optional.hpp>

namespace metatron::shape {
	auto Sphere::bounding_box(usize idx) const -> math::Bounding_Box {
		return {{-1.f}, {1.f}};
	}

	auto Sphere::operator()(
		math::Ray const& r,
		math::Vector<f32, 3> const& np,
		usize idx
	) const -> std::optional<Interaction> {
		auto a = math::dot(r.d, r.d);
		auto b = math::dot(r.o, r.d) * 2.f;
		auto c = math::dot(r.o, r.o) - 1.f;

		auto delta = b * b - 4.f * a * c;
		if (delta < 0.f) return {};

		auto x0 = (-b - math::sqrt(delta)) / (2.f * a);
		auto x1 = (-b + math::sqrt(delta)) / (2.f * a);
		if (x1 < 0.f) return {};

		auto t = x0 < 0.f ? x1 : x0;
		auto p = r.o + t * r.d;
		auto n = p;

		auto s = math::cartesion_to_sphere(p);
		auto& theta = s[0];
		auto& phi = s[1];
		auto uv = math::Vector<f32, 2>{
			theta / math::pi,
			phi / (2.f * math::pi)
		};

		auto dpdu = math::Vector<f32, 3>{
			std::cos(theta) * std::cos(phi),
			-std::sin(theta),
			std::cos(theta) * std::sin(phi),
		} / math::pi;
		auto dpdv = math::Vector<f32, 3>{
			-std::sin(theta) * std::sin(phi),
			0.f,
			std::sin(theta) * std::cos(phi),
		} / (2.f * math::pi);
		auto dndu = dpdu;
		auto dndv = dpdv;

		return Interaction{p, n, uv, t, dpdu, dpdv, dndu, dndv, 1.f};
	}

	auto Sphere::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u,
		usize idx
	) const -> std::optional<Interaction> {
		auto d = math::length(ctx.r.o);
		if (d < 1.f) {
			auto distr = math::Sphere_Distribution{};
			auto p = distr.sample(u);
			auto dir = math::normalize(p - ctx.r.o);

			OPTIONAL_OR_RETURN(intr, (*this)({ctx.r.o, dir}), {});
			intr.pdf = distr.pdf();
			return intr;
		} else {
			auto cos_theta_max = math::sqrt(1.f - 1.f / (d * d));
			auto distr = math::Cone_Distribution{cos_theta_max};
			auto dir = math::normalize(-ctx.r.o);

			auto sdir = distr.sample(u);
			auto rot = math::Quaternion<f32>::from_rotation_between({0.f, 0.f, 1.f}, dir);
			sdir = math::rotate(math::expand(sdir, 0.f), rot);

			OPTIONAL_OR_RETURN(intr, (*this)({ctx.r.o, sdir}), {});
			intr.pdf = distr.pdf();
			return intr;
		}
	}

}
