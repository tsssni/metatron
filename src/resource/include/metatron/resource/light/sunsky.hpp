#pragma once
#include <metatron/resource/light/light.hpp>

namespace mtt::light {
	struct Sunsky_Light final {
		Sunsky_Light(
			math::Vector<f32, 2> direction,
			f32 turbidity,
			f32 albedo
		) noexcept;

		auto static init() noexcept -> void;

		auto operator()(
			eval::Context const& ctx
		) const noexcept -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const noexcept -> std::optional<Interaction>;
		auto flags() const noexcept -> Flags;

	private:
		std::vector<f32> static sky_params;
		std::vector<f32> static sky_radiance;
		std::vector<f32> static sun_radiance;
		std::vector<f32> static sun_limb;
	};
}
