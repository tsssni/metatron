#pragma once
#include <metatron/core/math/ray.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/eval/context.hpp>
#include <memory>

namespace metatron::light {
	struct Interaction final {
		std::unique_ptr<spectra::Stochastic_Spectrum> Le;
		math::Vector<f32, 3> wi;
		math::Vector<f32, 3> p;
		f32 pdf;
	};

	struct Light {
		auto virtual operator()(math::Ray const& r, spectra::Stochastic_Spectrum const& Lo) const -> std::optional<std::unique_ptr<spectra::Stochastic_Spectrum>> = 0;
		auto virtual sample(eval::Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction> = 0;
	};
}
