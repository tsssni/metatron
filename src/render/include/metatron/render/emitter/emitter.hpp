#pragma once
#include <metatron/render/light/light.hpp>
#include <metatron/core/math/transform.hpp>

namespace metatron::emitter {
	struct Divider final {
		light::Light const* light;
		math::Transform const* local_to_world;
	};

	struct Interaction final {
		Divider const* divider;
		f32 pdf{0.f};
	};

	struct Emitter {
		auto virtual operator()(
			eval::Context const& ctx,
			Divider const& divider
		) const -> std::optional<emitter::Interaction> = 0;
		auto virtual sample(
			eval::Context const& ctx,
			f32 u
		) const -> std::optional<emitter::Interaction> = 0;
		auto virtual sample_infinite(
			eval::Context const& ctx,
			f32 u
		) const -> std::optional<emitter::Interaction> = 0;
	};
}
