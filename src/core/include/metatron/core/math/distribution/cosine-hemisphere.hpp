#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/distribution/disk.hpp>

namespace metatron::math {
	struct Cosine_Hemisphere_Distribution final {
		Cosine_Hemisphere_Distribution() = default;

		auto static sample(math::Vector<f32, 2> const& u) -> math::Vector<f32, 3> {
			auto d = Disk_Distribution::sample(u);
			return {d[0], std::sqrt(1.f - d[0] * d[0] - d[1] * d[1]), d[1]};
		}

		auto static pdf(f32 cos_theta) {
			return cos_theta / math::pi;
		}
	};
}
