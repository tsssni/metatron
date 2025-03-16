#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <memory>

namespace metatron::material {
	using Spectrum = spectra::Stochastic_Spectrum;

	namespace bsdf {
		struct Context final {
			Spectrum const* Lo;
			math::Vector<f32, 3> wo;
		};

		struct Interaction final {
			std::unique_ptr<Spectrum> f;
			math::Vector<f32, 3> wi;
			f32 pdf;
		};
	}

	struct Bsdf {
		auto virtual operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi,
			f32 lambda
		) const -> f32 = 0;
		auto virtual sample(bsdf::Context const& ctx, math::Vector<f32, 3> const& u) const -> std::optional<bsdf::Interaction> = 0;
	};
}
