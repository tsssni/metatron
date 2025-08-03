#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/shape/shape.hpp>

namespace mtt::light {
	struct Area_Light final {
		Area_Light(
			view<shape::Shape> shape,
			usize primitive = 0uz
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
		view<shape::Shape> shape;
		usize primitive;
	};
}
