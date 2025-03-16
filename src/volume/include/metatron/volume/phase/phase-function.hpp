#pragma once
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/math/vector.hpp>
#include <memory>

namespace metatron::phase {
	struct Context final {
		spectra::Stochastic_Spectrum const* Lo;
		math::Vector<f32, 3> wo;
	};

	struct Interaction final {
		std::unique_ptr<spectra::Stochastic_Spectrum> f;
		math::Vector<f32, 3> wi;
		f32 pdf;
	};

	struct Phase_Function {
		virtual ~Phase_Function() {}
		auto virtual operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi
		) const -> f32 = 0;
		auto virtual sample(Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction> = 0;
	};
}
