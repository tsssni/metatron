#pragma once
#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/core/math/complex.hpp>

namespace mtt::bsdf {
	struct Microfacet_Bsdf final {
		auto operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi
		) const noexcept -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 3> const& u
		) const noexcept -> std::optional<Interaction>;
		auto configure(Attribute const& attr) noexcept -> void;
		auto flags() const noexcept -> Flags;
		auto degrade() noexcept -> bool;

	private:
		auto fresnel(f32 cos_theta_i) const noexcept -> spectra::Stochastic_Spectrum;
		auto trowbridge_reitz(math::Vector<f32, 3> const& wm) const noexcept -> f32;
		auto visible_trowbridge_reitz(math::Vector<f32, 3> const& wo, math::Vector<f32, 3> const& wm) const noexcept -> f32;
		auto lambda(math::Vector<f32, 3> const& wo) const noexcept -> f32;
		auto smith_mask(math::Vector<f32, 3> const& wo) const noexcept -> f32;
		auto smith_shadow(math::Vector<f32, 3> const& wo, math::Vector<f32, 3> const& wi) const noexcept -> f32;
		auto torrance_sparrow(
			bool reflective, f32 pr, f32 pt,
			spectra::Stochastic_Spectrum const& F, f32 D, f32 G,
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi,
			math::Vector<f32, 3> const& wm
		) const noexcept -> std::optional<Interaction>;

		f32 alpha_u;
		f32 alpha_v;
		spectra::Stochastic_Spectrum eta;
		spectra::Stochastic_Spectrum k;
	};
}
