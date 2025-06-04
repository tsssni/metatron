#include <metatron/resource/shape/plane.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::shape {
	Plane::Plane(f32 a, f32 b, f32 c, f32 d):
		a(a), b(b), c(c), d(d) {}

	Plane::Plane(math::Vector<f32, 3> const& p, math::Vector<f32, 3> const& n):
		a(n[0]), b(n[1]), c(n[2]), d(-math::dot(n, p)) {}

	auto Plane::size() const -> usize {
		return 1uz;
	}

	auto Plane::bounding_box(usize idxuz) const -> math::Bounding_Box {
		return {};
	}

	auto Plane::operator()(
		math::Ray const& r,
		math::Vector<f32, 3> const& np,
		usize idx
	) const -> std::optional<Interaction> {
		auto n = math::Vector<f32, 3>{a, b, c};
		auto no = math::dot(n, r.o) + d;
		auto nd = math::dot(n, r.d);
		if (nd == 0.f && no != 0.f) {
			return {};
		}

		auto t = -no / nd;
		auto p = r.o + t * r.d;
		return Interaction{p, n, {}, t};
	}

	auto Plane::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u,
		usize idxuz
	) const -> std::optional<Interaction> {
		return {};
	}

}
