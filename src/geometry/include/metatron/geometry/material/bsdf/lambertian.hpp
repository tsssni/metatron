#pragma once
#include <metatron/geometry/material/bsdf/bsdf.hpp>
#include <memory>

namespace metatron::material {
	struct Lambertian_Bsdf final: Bsdf {
		Lambertian_Bsdf(
			std::unique_ptr<spectra::Stochastic_Spectrum> R,
			std::unique_ptr<spectra::Stochastic_Spectrum> T
		);
		auto operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi,
			spectra::Stochastic_Spectrum const& L
		) const -> std::optional<bsdf::Interaction>;
		auto sample(eval::Context const& ctx, math::Vector<f32, 3> const& u) const -> std::optional<bsdf::Interaction>;

	private:
		std::unique_ptr<spectra::Stochastic_Spectrum> R;
		std::unique_ptr<spectra::Stochastic_Spectrum> T;
	};
}
