#pragma once
#include <metatron/core/math/vector.hpp>

namespace metatron::math {
	namespace filter {
		struct Interaction final {
			Vector<f32, 2> p;
			f32 weight;
			f32 pdf;
		};
	}

	struct Filter {
		virtual ~Filter() {}
		auto virtual operator()(Vector<f32, 2> const& p) const -> f32 = 0;
		auto virtual sample(Vector<f32, 2> const& u) const -> std::optional<filter::Interaction> = 0;
	};
}
