#pragma once
#include <metatron/render/light/light.hpp>

namespace metatron::light {
	struct Emitter {
		auto emit(math::Ray const& r) const -> std::unique_ptr<spectra::Spectrum>;
		auto sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> Interaction;
	};
}
