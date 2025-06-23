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
		auto fresnel(f32 cos_theta_i) const -> spectra::Stochastic_Spectrum;
		auto trowbridge_reitz(math::Vector<f32, 3> const& wm) const -> f32;
		auto visible_trowbridge_reitz(math::Vector<f32, 3> const& wo, math::Vector<f32, 3> const& wm) const -> f32;
		auto lambda(math::Vector<f32, 3> const& wo) const -> f32;
		auto smith_mask(math::Vector<f32, 3> const& wo) const -> f32;
		auto smith_shadow(math::Vector<f32, 3> const& wo, math::Vector<f32, 3> const& wi) const -> f32;
		auto torrance_sparrow(
			bool reflective, f32 pr, f32 pt,
			spectra::Stochastic_Spectrum const& F, f32 D, f32 G,
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi,
			math::Vector<f32, 3> const& wm
		) const -> std::optional<Interaction>;

		f32 alpha_x;
		f32 alpha_y;
		spectra::Stochastic_Spectrum eta;
		spectra::Stochastic_Spectrum k;
	};
}
