#pragma once
#include <metatron/resource/bsdf/bsdf.hpp>

namespace mtt::bsdf {
	struct Lambertian_Bsdf final {
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
		spectra::Stochastic_Spectrum spectrum;
		spectra::Stochastic_Spectrum reflectance;
		spectra::Stochastic_Spectrum transmittance;
	};
}
