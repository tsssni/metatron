#pragma once
#include <metatron/render/light/light.hpp>

namespace mtt::light {
	struct Parallel_Light final: Light {
		Parallel_Light(view<spectra::Spectrum> L);

		auto operator()(
			eval::Context const& ctx
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const -> std::optional<Interaction>;
		auto flags() const -> Flags;

	private:
		view<spectra::Spectrum> L;
	};
}
