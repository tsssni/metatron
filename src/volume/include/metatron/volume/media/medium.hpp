#pragma once
#include <metatron/core/math/ray.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <memory>

namespace metatron::media {
	struct Context final {
		spectra::Stochastic_Spectrum const* Lo;
		math::Ray r;
		f32 t_max;
	};

	struct Interaction final {
		math::Vector<f32, 3> p;
		f32 pdf;
		std::unique_ptr<spectra::Stochastic_Spectrum> transmittance;
		std::unique_ptr<spectra::Stochastic_Spectrum> sigma_a;
		std::unique_ptr<spectra::Stochastic_Spectrum> sigma_s;
		std::unique_ptr<spectra::Stochastic_Spectrum> sigma_n;
	};

	struct Medium {
		auto virtual sample(Context const& ctx, f32 u) const -> std::optional<Interaction> = 0;
	};
}
