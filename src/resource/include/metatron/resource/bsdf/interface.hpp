#pragma once
#include <metatron/resource/bsdf/bsdf.hpp>

namespace mtt::bsdf {
	struct Interface_Bsdf final: Bsdf {
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
		spectra::Stochastic_Spectrum spectrum;
	};
}
