#pragma once
#include <metatron/render/light/light.hpp>

namespace metatron::light {
	namespace emitter {
		struct Interaction final {
			Light const* light;
			f32 pdf{0.f};
		};
	}

	struct Emitter {
		auto virtual operator()(Light const& light) const -> std::optional<emitter::Interaction> = 0;
		auto virtual sample(eval::Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<emitter::Interaction> = 0;
		auto virtual sample_infinite(eval::Context const& ctx, f32 u) const -> std::optional<emitter::Interaction> = 0;
	};
}
