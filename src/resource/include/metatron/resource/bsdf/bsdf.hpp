#pragma once
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/eval/context.hpp>
#include <metatron/core/math/vector.hpp>
#include <memory>

namespace metatron::bsdf {
	using Spectrum = spectra::Stochastic_Spectrum;

	struct Attribute final {
		spectra::Stochastic_Spectrum spectrum;
		spectra::Stochastic_Spectrum reflectance;
		spectra::Stochastic_Spectrum transmittance;
		f32 roughness{1.f};
		f32 metallic{0.f};
	};

	struct Interaction final {
		spectra::Stochastic_Spectrum f;
		math::Vector<f32, 3> wi;
		f32 pdf;
	};

	struct Bsdf {
		virtual ~Bsdf() {}
		auto virtual operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi
		) const -> std::optional<Interaction> = 0;
		auto virtual sample(
			eval::Context const& ctx,
			math::Vector<f32, 3> const& u
		) const -> std::optional<Interaction> = 0;
		auto virtual clone(Attribute const& attr) const -> std::unique_ptr<Bsdf> = 0;
		auto static is_interface(Bsdf const* bsdf) -> bool;
	};
}
