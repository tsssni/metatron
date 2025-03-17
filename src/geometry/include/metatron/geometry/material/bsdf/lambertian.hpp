#pragma once
#include <metatron/geometry/material/bsdf/bsdf.hpp>

namespace metatron::material {
	struct Lambertian_Bsdf final: Bsdf {
		Lambertian_Bsdf(std::unique_ptr<spectra::Spectrum> R, std::unique_ptr<spectra::Spectrum> T);
		auto operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi,
			f32 lambda
		) const -> f32;
		auto sample(eval::Context const& ctx, math::Vector<f32, 3> const& u) const -> std::optional<bsdf::Interaction>;

	private:
		std::unique_ptr<spectra::Spectrum> R;
		std::unique_ptr<spectra::Spectrum> T;
	};
}
