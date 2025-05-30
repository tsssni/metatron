#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::math {
	struct Bounding_Box final {
		Vector<f32, 3> p_min{high<f32>};
		Vector<f32, 3> p_max{low<f32>};
	};

	auto inline inside(math::Vector<f32, 3> const& p, Bounding_Box const& bbox) -> bool {
		auto in = true;
		for (auto i = 0uz; i < 3; i++) {
			in = in
			&& p[i] - bbox.p_min[i] > epsilon<f32>
			&& -p[i] + bbox.p_max[i] > epsilon<f32>;
		}
		return in;
	}

	auto inline hit(Ray const& r, Bounding_Box const& bbox) -> std::optional<f32> {
		auto hit_min = (bbox.p_min - r.o) / r.d;
		auto hit_max = (bbox.p_max - r.o) / r.d;
		for (auto i = 0uz; i < 3uz; i++) {
			if (std::abs(r.d[i]) < epsilon<f32>) {
				hit_min[i] = -inf<f32>;
				hit_max[i] = +inf<f32>;
			}

			if (hit_min[i] > hit_max[i]) {
				std::swap(hit_min[i], hit_max[i]);
			}
		}

		auto t_enter = max(hit_min);
		auto t_exit = min(hit_max);
		if (t_exit < -epsilon<f32> || t_enter > t_exit) {
			return {};
		} else {
			return t_enter > 0.f ? t_enter : t_exit;
		}
	}
}
