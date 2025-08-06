#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <variant>

namespace mtt::compo {
	struct Parallel_Light final {
		ecs::Entity spectrum;
		i32 parallel{0};
	};

	struct Point_Light final {
		ecs::Entity spectrum;
		i32 point{0};
	};

	struct Spot_Light final {
		ecs::Entity spectrum;
		f32 falloff_start_theta;
		f32 falloff_end_theta;
		i32 spot{0};
	};

	struct Environment_Light final {
		ecs::Entity env_map;
		i32 environment{0};
	};

	using Light = std::variant<
		Parallel_Light,
		Point_Light,
		Spot_Light,
		Environment_Light
	>;
}
