#pragma once
#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/core/math/complex.hpp>

namespace metatron::bsdf {
	struct Microfacet_Bsdf final: Bsdf {
		auto operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 3> const& u
		) const -> std::optional<Interaction>;
		auto clone(Attribute const& attr) const -> std::unique_ptr<Bsdf>;
		auto flags() const -> Flags;

	private:
		spectra::Stochastic_Spectrum interior_eta;
		spectra::Stochastic_Spectrum exterior_eta;
		spectra::Stochastic_Spectrum interior_k;
		spectra::Stochastic_Spectrum exterior_k;
		f32 u_roughness;
		f32 v_roughness;
	};
}
