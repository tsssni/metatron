#pragma once
#include <metatron/resource/bsdf/bsdf.hpp>
#include <memory>

namespace metatron::bsdf {
	struct Lambertian_Bsdf final: Bsdf {
		Lambertian_Bsdf(
			std::unique_ptr<spectra::Stochastic_Spectrum> R,
			std::unique_ptr<spectra::Stochastic_Spectrum> T
		);
		auto operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi,
			spectra::Stochastic_Spectrum const& L
		) const -> std::optional<Interaction>;
		auto sample(eval::Context const& ctx, math::Vector<f32, 3> const& u) const -> std::optional<Interaction>;

	private:
		std::unique_ptr<spectra::Stochastic_Spectrum> R;
		std::unique_ptr<spectra::Stochastic_Spectrum> T;
	};
}
