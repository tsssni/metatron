#pragma once
#include <metatron/volume/media/medium.hpp>
#include <metatron/volume/phase/phase-function.hpp>

namespace metatron::media {
	struct Vaccum_Medium final: Medium {
		Vaccum_Medium();
		auto sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction>;
	};
}
