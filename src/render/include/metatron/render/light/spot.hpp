#pragma once
#include <metatron/render/light/light.hpp>

namespace mtt::light {
	struct Spot_Light final: Light {
		Spot_Light(
			pro::proxy_view<spectra::Spectrum> L,
			f32 falloff_start_theta,
			f32 falloff_end_theta
		);

		auto operator()(
			eval::Context const& ctx
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const -> std::optional<Interaction>;
		auto flags() const -> Flags;

	private:
		pro::proxy_view<spectra::Spectrum> L;
		f32 falloff_start_cos_theta;
		f32 falloff_end_cos_theta;
	};
}
