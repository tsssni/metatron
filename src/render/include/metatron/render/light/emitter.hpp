#pragma once
#include <metatron/render/light/light.hpp>

namespace metatron::light {
	namespace emitter {
		struct Interaction final {
			Light const* light;
			f32 pdf;
		};
	}

	struct Emitter {
		auto virtual operator()(math::Ray const& r) const -> std::optional<Light const*> = 0;
		auto virtual sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<emitter::Interaction> = 0;
	};
}
