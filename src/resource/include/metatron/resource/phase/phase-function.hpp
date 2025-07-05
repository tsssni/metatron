#pragma once
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/eval/context.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::phase {
	struct Attribute final {
		spectra::Stochastic_Spectrum spectrum;
	};

	struct Interaction final {
		spectra::Stochastic_Spectrum f;
		math::Vector<f32, 3> wi;
		f32 pdf;
	};

	struct Phase_Function {
		virtual ~Phase_Function() {}
		auto virtual operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi
		) const -> std::optional<Interaction> = 0;
		auto virtual sample(eval::Context const& ctx, math::Vector<f32, 2> const& u) const -> std::optional<Interaction> = 0;
		auto virtual clone(Attribute const& attr) const -> std::unique_ptr<Phase_Function> = 0;
	};
}
