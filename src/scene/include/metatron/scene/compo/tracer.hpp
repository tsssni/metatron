#pragma once
#include <metatron/scene/ecs/entity.hpp>

namespace mtt::compo {
	struct Divider final {
		ecs::Entity shape;
		ecs::Entity medium;
		ecs::Entity material;
	};

	enum struct Emitter: i32 {
		uniform,
	};

	enum struct Acceleration: i32 {
		lbvh,
	};

	enum struct Integrator: i32 {
		volume_path,
	};

	struct Tracer final {
		Emitter emitter;
		Acceleration accel;
		Integrator integrator;
	};
}
