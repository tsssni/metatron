#pragma once
#include <metatron/render/light/light.hpp>

namespace metatron::light {
	struct Parallel_Light final: Light {
		Parallel_Light(
			std::unique_ptr<spectra::Spectrum> L,
			math::Vector<f32, 3> const& sphere_coord
		);

		auto operator()(
			eval::Context const& ctx
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const -> std::optional<Interaction>;

	private:
		std::unique_ptr<spectra::Spectrum> L;
		math::Vector<f32, 3> sphere_coord;
	};
}
