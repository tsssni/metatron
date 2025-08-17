#pragma once
#include <metatron/scene/ecs/entity.hpp>

namespace mtt::compo {
	struct Divider final {
		ecs::Entity shape;
		ecs::Entity int_medium = "/hierarchy/medium/vaccum"_et;
		ecs::Entity ext_medium = "/hierarchy/medium/vaccum"_et;
		ecs::Entity material;
	};

	struct Uniform_Emitter final {
		i32 uniform{0};
	};

	using Emitter = std::variant<
		Uniform_Emitter
	>;

	struct LBVH final {
		i32 lbvh{0};
	};

	using Acceleration = std::variant<
		LBVH
	>;

	struct Volume_Path_Integrator final {
		i32 volume_path{0};
	};

	using Integrator = std::variant<
		Volume_Path_Integrator
	>;

	struct Tracer final {
		Emitter emitter = Uniform_Emitter{};
		Acceleration accel = LBVH{};
		Integrator integrator = Volume_Path_Integrator{};
	};
}
