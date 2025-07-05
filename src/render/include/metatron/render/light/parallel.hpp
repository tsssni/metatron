#pragma once
#include <metatron/render/light/light.hpp>

namespace mtt::light {
	struct Parallel_Light final: Light {
		Parallel_Light(std::unique_ptr<spectra::Spectrum> L);

		auto operator()(
			eval::Context const& ctx
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const -> std::optional<Interaction>;
		auto flags() const -> Flags;

	private:
		std::unique_ptr<spectra::Spectrum> L;
	};
}
