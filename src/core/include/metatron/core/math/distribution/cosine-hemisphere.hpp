#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/distribution/disk.hpp>

namespace metatron::math {
	struct Cosine_Hemisphere_Distribution final {
		Cosine_Hemisphere_Distribution() = default;

		auto sample(math::Vector<f32, 2> const& u) -> math::Vector<f32, 3> {
			auto dist = Disk_Distribution{};
			auto d = dist.sample(u);
			return {d[0], std::sqrt(1.f - d[0] * d[0] - d[1] * d[1]), d[1]};
		}

		auto pdf(f32 cos_theta) {
			return cos_theta / math::pi;
		}
	};
}
