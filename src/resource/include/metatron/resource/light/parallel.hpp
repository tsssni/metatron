#pragma once
#include <metatron/resource/light/light.hpp>

namespace mtt::light {
	struct Parallel_Light final {
		view<spectra::Spectrum> L;
		// FIXME: could not use single proxy_view to construct
		// Parallel_Light(view<spectra::Spectrum> L) noexcept;

		auto operator()(
			eval::Context const& ctx
		) const noexcept -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const noexcept -> std::optional<Interaction>;
		auto flags() const noexcept -> Flags;

	private:
		// view<spectra::Spectrum> L;
	};
}
