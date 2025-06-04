#pragma once
#include <metatron/render/light/light.hpp>
#include <metatron/resource/shape/shape.hpp>

namespace metatron::light {
	struct Area_Light final: Light {
		Area_Light(
			shape::Shape const& shape,
			usize primitive = 0uz
		);

		auto operator()(
			eval::Context const& ctx
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u
		) const -> std::optional<Interaction>;

	private:
		shape::Shape const* shape;
		usize primitive;
	};
}
