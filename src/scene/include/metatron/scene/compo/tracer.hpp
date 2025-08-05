#pragma once
#include <metatron/scene/ecs/entity.hpp>

namespace mtt::compo {
	struct Divider final {
		ecs::Entity shape;
		ecs::Entity medium;
		ecs::Entity material;
	};

	struct Uniform_Emitter final {
		std::string_view uniform = "";
	};

	using Emitter = std::variant<
		Uniform_Emitter
	>;

	struct LBVH final {
		std::string_view lbvh = "";
	};

	using Acceleration = std::variant<
		LBVH
	>;

	struct Volume_Path_Integrator final {
		std::string_view volume_path = "";
	};

	using Integrator = std::variant<
		Volume_Path_Integrator
	>;

	struct Tracer final {
		Emitter emitter;
		Acceleration accel;
		Integrator integrator;
	};
}
