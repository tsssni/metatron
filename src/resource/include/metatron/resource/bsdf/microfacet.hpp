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
		auto degrade() -> bool;

	private:
		auto fresnel(f32 cos_theta_i, math::Complex<f32> eta) const -> f32;
		auto trowbridge_reitz(math::Vector<f32, 3> const& wm) const -> f32;
		auto visible_trowbridge_reitz(math::Vector<f32, 3> const& wo, math::Vector<f32, 3> const& wm) const -> f32;
		auto lambda(math::Vector<f32, 3> const& wo) const -> f32;
		auto smith_mask(math::Vector<f32, 3> const& wo) const -> f32;
		auto smith_shadow(math::Vector<f32, 3> const& wo, math::Vector<f32, 3> const& wi) const -> f32;
		auto torrance_sparrow(
			bool reflective, f32 pr, f32 pt,
			f32 F, f32 D, f32 G,
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi,
			math::Vector<f32, 3> const& wm
		) const -> std::optional<Interaction>;

		spectra::Stochastic_Spectrum interior_eta;
		spectra::Stochastic_Spectrum exterior_eta;
		spectra::Stochastic_Spectrum interior_k;
		spectra::Stochastic_Spectrum exterior_k;
		f32 u_roughness;
		f32 v_roughness;

		math::Complex<f32> eta;
	};
}
