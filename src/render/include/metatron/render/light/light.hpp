#pragma once
#include <metatron/core/math/ray.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <memory>

namespace metatron::light {
	using Spectrum = std::unique_ptr<spectra::Spectrum>;

	struct Context final {
		math::Vector<f32, 3> p;
		math::Vector<f32, 3> n;
	};

	struct Interaction final {
		Spectrum L;
		math::Vector<f32, 3> wi;
		f32 pdf;
	};

	struct Light {
		auto virtual operator()(math::Ray const& r) const -> std::optional<Spectrum> = 0;
		auto virtual sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction> = 0;
	};
}
