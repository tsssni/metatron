#pragma once
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/eval/context.hpp>
#include <metatron/core/math/vector.hpp>
#include <memory>
#include <unordered_map>

namespace metatron::bsdf {
	using Spectrum = spectra::Stochastic_Spectrum;

	struct Attribute final {
		std::unordered_map<std::string, spectra::Stochastic_Spectrum> spectra;
		std::unordered_map<std::string, math::Vector<f32, 4>> vectors;
		bool inside;
	};

	struct Interaction final {
		spectra::Stochastic_Spectrum f;
		math::Vector<f32, 3> wi;
		f32 pdf;
		bool degraded{false};
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
		auto virtual degrade() -> bool = 0;
	};
}
