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
		spectra::Stochastic_Spectrum eta;
		spectra::Stochastic_Spectrum k;
		f32 u_roughness{1.f};
		f32 v_roughness{1.f};
	};

	struct Interaction final {
		spectra::Stochastic_Spectrum f;
		math::Vector<f32, 3> wi;
		f32 pdf;
	};

	struct Bsdf {
		enum Flags {
			reflective = 1 << 0,
			transmissive = 1 << 1,
			interface = 1 << 2,
		};

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
		auto virtual flags() const -> Flags = 0;
	};
}
