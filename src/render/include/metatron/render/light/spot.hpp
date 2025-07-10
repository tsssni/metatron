#pragma once
#include <metatron/render/light/light.hpp>

namespace mtt::light {
	struct Spot_Light final {
		Spot_Light(
			view<spectra::Spectrum> L,
			f32 falloff_start_theta,
			f32 falloff_end_theta
		) noexcept;

		auto operator()(
			eval::Context const& ctx
		) const noexcept -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const noexcept -> std::optional<Interaction>;
		auto flags() const noexcept -> Flags;

	private:
		view<spectra::Spectrum> L;
		f32 falloff_start_cos_theta;
		f32 falloff_end_cos_theta;
	};
}
