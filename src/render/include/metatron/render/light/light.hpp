#pragma once
#include <metatron/core/math/ray.hpp>
#include <metatron/render/spectra/spectrum.hpp>
#include <memory>

namespace metatron::light {
	struct Context final {
		math::Vector<f32, 3> p;
		math::Vector<f32, 3> n;
	};

	struct Interaction final {
		std::unique_ptr<spectra::Spectrum> L;
		math::Vector<f32, 3> wi;
		f32 pdf;
	};

	struct Light {
		auto virtual emit(math::Ray const& r) const -> std::unique_ptr<spectra::Spectrum> = 0;
		auto virtual sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> Interaction = 0;
	};
}
