#pragma once
#include <metatron/render/light/light.hpp>

namespace metatron::light {
	struct Point_Light final: Light {
		Point_Light(std::unique_ptr<spectra::Spectrum> L);

		auto operator()(
			eval::Context const& ctx
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const -> std::optional<Interaction>;

	private:
		std::unique_ptr<spectra::Spectrum> L;
	};
}
