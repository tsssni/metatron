#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace metatron::math {
	struct Disk_Distribution final {
		Disk_Distribution() = default;

		auto sample(Vector<f32, 2> const& u) const -> Vector<f32, 2> {
			auto disk_u = u * 2.f - 1.f;
			auto r = 0.f;
			auto theta = 0.f;

			if (abs(disk_u) < Vector<f32, 2>{epsilon<f32>}) {
				return {0.f};
			} else if (math::abs(disk_u[0]) > math::abs(disk_u[1])) {
				r = math::abs(disk_u[0]);
				theta = pi / 4.f * guarded_div(disk_u[1], disk_u[0]);
			} else {
				r = math::abs(disk_u[1]);
				theta = pi / 2.f - pi / 4.f * guarded_div(disk_u[0], disk_u[1]);
			}

			return r * Vector<f32, 2>{std::cos(theta), std::sin(theta)};
		}

		auto pdf() const -> f32 {
			return 1.f / pi;
		}
	};
}
