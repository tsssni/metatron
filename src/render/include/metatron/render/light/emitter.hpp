#pragma once
#include <metatron/render/light/light.hpp>

namespace metatron::light {
	struct Emitter {
		auto virtual operator()(math::Ray const& r) const -> std::optional<Interaction> = 0;
		auto virtual sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction> = 0;
	};
}
