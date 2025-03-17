#pragma once
#include <metatron/volume/phase/phase-function.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/eval/context.hpp>
#include <memory>

namespace metatron::media {
	struct Interaction final {
		math::Vector<f32, 3> p;
		phase::Phase_Function const* phase;
		f32 t;
		f32 pdf;
		std::unique_ptr<spectra::Stochastic_Spectrum> transmittance;
		std::unique_ptr<spectra::Stochastic_Spectrum> sigma_a;
		std::unique_ptr<spectra::Stochastic_Spectrum> sigma_s;
		std::unique_ptr<spectra::Stochastic_Spectrum> sigma_n;
		std::unique_ptr<spectra::Stochastic_Spectrum> Le;
	};

	struct Medium {
		auto virtual sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction> = 0;
	};
}
