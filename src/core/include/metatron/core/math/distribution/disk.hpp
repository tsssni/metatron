#pragma once
#include <metatron/core/math/vector.hpp>

namespace metatron::math {
	struct Disk_Distribution final {
		Disk_Distribution() = default;

		auto sample(math::Vector<f32, 2> const& u) -> math::Vector<f32, 2> {
			auto disk_u = u * 2.f - 1.f;
			auto r = 0.f;
			auto theta = 0.f;

			if (disk_u == math::Vector<f32, 2>{0.f}) {
				return {0.f};
			} else if (std::abs(disk_u[0]) > std::abs(disk_u[1])) {
				r = disk_u[0];
				theta = math::pi / 4.f * disk_u[1] / disk_u[0];
			} else {
				r = disk_u[1];
				theta = math::pi / 2.f - math::pi / 4.f * disk_u[0] / disk_u[1];
			}

			return r * math::Vector<f32, 2>{std::cos(theta), std::sin(theta)};
		}

		auto pdf() -> f32 {
			return 1.f / pi;
		}
	};
}
